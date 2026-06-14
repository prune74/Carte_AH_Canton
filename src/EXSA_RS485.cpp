/*
 * Module : EXSA_RS485
 * Rôle   : Gestion du bus RS485 half‑duplex pour la communication EXSA ↔ SA.
 *
 * Fonctionnement général :
 *   - Utilise un transceiver RS485 avec broche DE/RE commune.
 *   - En réception : DE/RE = LOW
 *   - En émission  : DE/RE = HIGH
 *
 *   - begin() configure :
 *        • l’UART matériel (HardwareSerial)
 *        • les broches RX/TX
 *        • la broche DE/RE
 *
 *   - send() :
 *        • active l’émission
 *        • envoie la trame UART
 *        • attend la fin réelle (flush)
 *        • repasse en réception
 *
 * Contraintes temps réel :
 *   - Aucun malloc
 *   - Aucun blocage long
 *   - Délais microsecondes uniquement (stabilisation transceiver)
 *
 * Notes :
 *   - Le half‑duplex impose que l’EXSA ne parle jamais pendant
 *     que le SA émet, d’où la gestion stricte de DE/RE.
 *   - Le code est volontairement minimaliste pour garantir
 *     une fiabilité maximale.
 */

#include "EXSA_RS485.h"
#include "EXSA_Pins.h"

#include <Arduino.h>

namespace
{
    // UART matériel utilisé pour le RS485
    HardwareSerial *g_uart = nullptr;

    // Broche DE/RE du transceiver RS485
    gpio_num_t g_pinDE_RE = GPIO_NUM_NC;
}

/* ============================================================
 * begin()
 * ------------------------------------------------------------
 * Initialise :
 *   - l’UART matériel (baudrate, format, RX/TX)
 *   - la broche DE/RE (LOW = réception)
 *
 * Le module démarre toujours en mode réception.
 * ============================================================ */
void EXSA_RS485::begin(HardwareSerial &serial,
                       uint32_t baudrate,
                       gpio_num_t pinDE_RE) noexcept
{
    g_uart = &serial;
    g_pinDE_RE = pinDE_RE;

    pinMode(g_pinDE_RE, OUTPUT);
    digitalWrite(g_pinDE_RE, LOW); // réception par défaut

    g_uart->begin(baudrate, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
}

/* ============================================================
 * uart()
 * ------------------------------------------------------------
 * Retourne une référence vers l’UART matériel.
 * Utilisé par EXSA_UartRx pour lire les trames SA.
 * ============================================================ */
HardwareSerial &EXSA_RS485::uart() noexcept
{
    return *g_uart;
}

/* ============================================================
 * send()
 * ------------------------------------------------------------
 * Envoie une trame RS485 en half‑duplex.
 *
 * Étapes :
 *   1) DE/RE = HIGH → passage en émission
 *   2) petit délai (stabilisation transceiver)
 *   3) écriture UART
 *   4) flush() → attendre la fin réelle de l’envoi
 *   5) petit délai
 *   6) DE/RE = LOW → retour en réception
 *
 * Le protocole Discovery 2026 exige que le bus soit libéré
 * immédiatement après l’envoi.
 * ============================================================ */
void EXSA_RS485::send(const uint8_t *data, uint8_t len) noexcept
{
    if (!g_uart)
        return;

    // Passage en émission
    digitalWrite(g_pinDE_RE, HIGH);
    delayMicroseconds(8);

    g_uart->write(data, len);
    g_uart->flush(); // attendre la fin réelle de l’envoi

    delayMicroseconds(8);

    // Retour en réception
    digitalWrite(g_pinDE_RE, LOW);
}
