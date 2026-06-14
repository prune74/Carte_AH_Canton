#include "EXSA_UartTx.h"
#include "EXSA_RS485.h"
#include "EXSA_Config.h"
#include "Discovery_Protocol.h"
#include "EXSA_Main.h"

#include <Arduino.h>
#include <HardwareSerial.h>

// Adresse EXSA (0 = Horaire, 1 = AntiHoraire)
extern uint8_t exsaAdresse;
extern bool exsaHasBooster;

/* ============================================================
 * envoyerPong()
 * ============================================================ */
void EXSA_UartTx::envoyerPong(uint8_t index)
{
    uint8_t frame[4] = {
        PROTO_SYNC_BYTE,
        PROTO_PONG,
        exsaAdresse,
        index
    };

    EXSA_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerTrameOccupation()
 * ============================================================ */
void EXSA_UartTx::envoyerTrameOccupation(bool occ)
{
    const uint8_t valeur = occ ? PROTO_OCC_ACTIVE : PROTO_OCC_LIBRE;

    uint8_t frame[4] = {
        PROTO_SYNC_BYTE,
        PROTO_04_OCCUPATION,
        exsaAdresse,
        valeur
    };

    EXSA_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerTrameDeltaAxe()
 * ============================================================ */
void EXSA_UartTx::envoyerTrameDeltaAxe(int delta)
{
    const uint8_t valeur = (delta > 0) ? PROTO_DELTA_PLUS_UN
                                       : PROTO_DELTA_MOINS_UN;

    uint8_t frame[4] = {
        PROTO_SYNC_BYTE,
        PROTO_05_DELTA_AXE,
        exsaAdresse,
        valeur
    };

    EXSA_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerTramePonctuel()
 * ============================================================ */
void EXSA_UartTx::envoyerTramePonctuel(bool actif)
{
    uint8_t valeur;

    if (exsaAdresse == 0) // Horaire
        valeur = actif ? PROTO_PONCT_H_ACTIVE : PROTO_PONCT_H_INACTIVE;
    else
        valeur = actif ? PROTO_PONCT_AH_ACTIVE : PROTO_PONCT_AH_INACTIVE;

    uint8_t frame[4] = {
        PROTO_SYNC_BYTE,
        PROTO_03_PONCTUEL,
        exsaAdresse,
        valeur
    };

    EXSA_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerTramePositionAiguille()
 * ============================================================ */
void EXSA_UartTx::envoyerTramePositionAiguille(uint8_t index,
                                               uint8_t position,
                                               uint8_t etat)
{
    uint8_t frame[6] = {
        PROTO_SYNC_BYTE,
        PROTO_06_POSITION_AIGUILLE,
        exsaAdresse,
        index,
        position,
        etat
    };

    EXSA_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerTrameBooster()
 * ============================================================ */
void EXSA_UartTx::envoyerTrameBooster(uint8_t etat,
                                      uint8_t courant,
                                      uint8_t tension)
{
    uint8_t present = exsaHasBooster ? 1 : 0;

    uint8_t frame[7] = {
        PROTO_SYNC_BYTE,
        PROTO_07_BOOSTER,
        exsaAdresse,
        etat,
        courant,
        tension,
        present
    };

    EXSA_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerTrameRailcom()
 * ============================================================ */
void EXSA_UartTx::envoyerTrameRailcom(uint8_t type,
                                      uint16_t adresse)
{
    uint8_t frame[6] = {
        PROTO_SYNC_BYTE,
        PROTO_08_RAILCOM_ADRESSE,
        exsaAdresse,
        type,
        uint8_t(adresse & 0xFF),
        uint8_t((adresse >> 8) & 0xFF)
    };

    EXSA_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerTrameCalibBooster()
 * ============================================================ */
void EXSA_UartTx::envoyerTrameCalibBooster(uint16_t libre,
                                           uint16_t occupe)
{
    uint8_t frame[7] = {
        PROTO_SYNC_BYTE,
        PROTO_09_CALIB_BOOSTER,
        exsaAdresse,
        uint8_t(libre & 0xFF),
        uint8_t(libre >> 8),
        uint8_t(occupe & 0xFF),
        uint8_t(occupe >> 8)
    };

    EXSA_RS485::send(frame, sizeof(frame));
}
