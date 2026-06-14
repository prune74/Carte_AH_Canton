/* ============================================================
   Module : EXSA_UartRx
   Rôle   : Réception et parsing du protocole SA → EXSA (RS485)
   Version : ENUM (Option A)
   ------------------------------------------------------------
   Fonctionnement général :
     - Le SA envoie des trames structurées :
         SYNC (0xAA)
         OPCODE
         DATA[n]

     - Le parser fonctionne comme une petite machine à états :
         0 → attente SYNC
         1 → lecture OPCODE
         2 → lecture DATA jusqu’à longueur attendue

     - Une fois la trame complète :
         → dispatch vers EXSA_Callbacks

   Contraintes :
     - Aucun malloc
     - Parsing non bloquant
     - Compatible ISR RS485 (via EXSA_RS485)
     - Très robuste aux erreurs (resetParser)
============================================================ */

#include "EXSA_UartRx.h"
#include "EXSA_RS485.h"
#include "EXSA_Pins.h"
#include "Discovery_Protocol.h"
#include "EXSA_UartTx.h"
#include "EXSA_Callbacks.h"
#include "EXSA_Calibration.h"

#include <Arduino.h>

static HardwareSerial *uart = nullptr;

/* ============================================================
   Buffer interne du parser
============================================================ */
static uint8_t rxState = 0;
static uint8_t rxOpcode = 0;
static uint8_t rxData[8];
static uint8_t rxIndex = 0;

/* ============================================================
   Initialisation UART
============================================================ */
void EXSA_UartRx::begin(HardwareSerial &serial, uint32_t baudrate) noexcept
{
    EXSA_RS485::begin(serial, baudrate, PIN_RS485_DE_RE);
    uart = &EXSA_RS485::uart();
}

/* ============================================================
   resetParser()
============================================================ */
static inline void resetParser() noexcept
{
    rxState = 0;
    rxOpcode = 0;
    rxIndex = 0;
}

/* ============================================================
   handleFrame()
   Dispatch vers EXSA_Callbacks selon l’opcode reçu.
============================================================ */
static inline void handleFrame(uint8_t opcode, uint8_t *data, uint8_t len) noexcept
{
    switch (opcode)
    {
    case PROTO_PING:
        EXSA_UartTx::envoyerPong(data[0]);
        break;

    case PROTO_E4_TOPOLOGIE_CAN:
        EXSA_Callbacks::onTopologie(data, len);
        break;

    case PROTO_E5_CONFIG_SIGNAUX:
        EXSA_Callbacks::onConfigSignaux(data, len);
        break;

    case PROTO_E6_ASPECT_HORAIRE:
        EXSA_Callbacks::onAspectHoraire(data[0]);
        break;

    case PROTO_E7_ASPECT_ANTIHORAIRE:
        EXSA_Callbacks::onAspectAntiHoraire(data[0]);
        break;

    case PROTO_E8_DIRECTION_HORAIRE:
        EXSA_Callbacks::onDirectionHoraire(data[0]);
        break;

    case PROTO_E9_DIRECTION_ANTIHORAIRE:
        EXSA_Callbacks::onDirectionAntiHoraire(data[0]);
        break;

    case PROTO_EA_OCCUPATION_VOISINS:
        EXSA_Callbacks::onOccupationVoisins(data[0]);
        break;

    case PROTO_F0_SERVO_MOVE:
        EXSA_Callbacks::onServoMove(data[0], data[1]);
        break;

    case PROTO_F1_SERVO_CONFIG:
        EXSA_Callbacks::onServoConfig(
            data[0],
            uint16_t(data[1] | (data[2] << 8)),
            uint16_t(data[3] | (data[4] << 8)),
            uint16_t(data[5] | (data[6] << 8)));
        break;

    case PROTO_F2_SERVO_TEST:
        EXSA_Callbacks::onServoTest(data[0]);
        break;

    /* ============================================================
       F3 — Recalibration automatique demandée par le SA
       ============================================================ */
    case PROTO_F3_RECALIBRER_BOOSTER:
#if EXSA_DEBUG
        Serial.println("[RX] Recalibration Booster demandée par SA");
#endif
        EXSA_Calibration::start();
        break;

    /* ============================================================
       F4 — Le SA renvoie les seuils stockés dans settings.json
       ============================================================ */
    case PROTO_F4_SET_SEUILS:
    {
        uint16_t libre = uint16_t(data[0] | (data[1] << 8));
        uint16_t occupe = uint16_t(data[2] | (data[3] << 8));

#if EXSA_DEBUG
        Serial.printf("[RX] SET SEUILS : libre=%u occupe=%u\n", libre, occupe);
#endif

        EXSA_Calibration::setSeuils(libre, occupe);
        break;
    }

    /* ============================================================
       F5 — Booster ON/OFF (SA → EXSA)
       ============================================================ */
    case PROTO_F5_BOOSTER_POWER:
    {
        uint8_t power = data[0]; // 0 = OFF, 1 = ON

#if EXSA_DEBUG
        Serial.printf("[RX] BOOSTER POWER : %s\n", power ? "ON" : "OFF");
#endif

        EXSA_Callbacks::onBoosterPower(power);
        break;
    }

    default:
        break;
    }
}

/* ============================================================
   process()
   Parsing non bloquant du protocole SA → EXSA
============================================================ */
void EXSA_UartRx::process() noexcept
{
    if (!uart)
        return;

    while (uart->available())
    {
        const uint8_t b = uart->read();

        switch (rxState)
        {
        /* ------------------------------
           État 0 : attente du SYNC
        ------------------------------ */
        case 0:
            if (b == PROTO_SYNC_BYTE)
                rxState = 1;
            break;

        /* ------------------------------
           État 1 : lecture OPCODE
        ------------------------------ */
        case 1:
            rxOpcode = b;
            rxIndex = 0;
            rxState = 2;
            break;

        /* ------------------------------
           État 2 : lecture DATA
        ------------------------------ */
        case 2:
        {
            rxData[rxIndex++] = b;

            uint8_t expectedLen = 0;

            switch (rxOpcode)
            {
            case PROTO_PING:
                expectedLen = 1;
                break;

            case PROTO_E4_TOPOLOGIE_CAN:
                expectedLen = 10;
                break;
            case PROTO_E5_CONFIG_SIGNAUX:
                expectedLen = 4;
                break;

            case PROTO_E6_ASPECT_HORAIRE:
                expectedLen = 1;
                break;
            case PROTO_E7_ASPECT_ANTIHORAIRE:
                expectedLen = 1;
                break;

            case PROTO_E8_DIRECTION_HORAIRE:
                expectedLen = 1;
                break;
            case PROTO_E9_DIRECTION_ANTIHORAIRE:
                expectedLen = 1;
                break;

            case PROTO_EA_OCCUPATION_VOISINS:
                expectedLen = 1;
                break;

            case PROTO_F0_SERVO_MOVE:
                expectedLen = 2;
                break;
            case PROTO_F1_SERVO_CONFIG:
                expectedLen = 7;
                break;
            case PROTO_F2_SERVO_TEST:
                expectedLen = 1;
                break;

            case PROTO_F3_RECALIBRER_BOOSTER:
                expectedLen = 0;
                break;

            case PROTO_F4_SET_SEUILS:
                expectedLen = 4;
                break;

            case PROTO_F5_BOOSTER_POWER:
                expectedLen = 1;
                break;

            default:
                expectedLen = 0;
                break;
            }

            // Trame complète ?
            if (rxIndex >= expectedLen)
            {
                handleFrame(rxOpcode, rxData, expectedLen);
                resetParser();
            }
            break;
        }

        default:
            resetParser();
            break;
        }
    }
}
