#pragma once
#include <stdint.h>
#include "Discovery_Protocol.h"

/*
 * EXSA_UartTx.h — Gestion des trames UART envoyées par l’EXSA vers le SA.
 *
 * Toutes les trames suivent le protocole Discovery 2026 :
 *   SYNC (0xAA)
 *   OPCODE
 *   DATA[n]
 *
 * RÈGLE OFFICIELLE (Discovery 2026 — RS485 multi‑EXSA) :
 * -----------------------------------------------------
 *   → Toutes les trames EXSA → SA commencent par :
 *
 *        [SYNC][OPCODE][exsaAdresse][DATA...]
 *
 *     où :
 *        exsaAdresse = 0 (EXSA Horaire)
 *        exsaAdresse = 1 (EXSA Anti‑Horaire)
 *
 *   → Cela permet au SA d’identifier immédiatement quel EXSA parle
 *     sur le bus RS485 partagé.
 *
 * L’EXSA envoie :
 *   - PONG
 *   - OCCUPATION
 *   - DELTA AXE
 *   - PONCTUEL
 *   - POSITION AIGUILLE
 *   - BOOSTER (état + télémétrie + présence booster)
 *   - RAILCOM (adresse détectée)
 *   - CALIBRATION BOOSTER (seuilLibre + seuilOccupe)
 */

class EXSA_UartTx
{
public:
    /* --- Supervision --- */
    static void envoyerPong(uint8_t index);

    /* --- Occupation canton --- */
    static void envoyerTrameOccupation(bool occ);

    /* --- Comptage essieux --- */
    static void envoyerTrameDeltaAxe(int delta);

    /* --- Capteurs ponctuels --- */
    static void envoyerTramePonctuel(bool actif);

    /* --- Position réelle des aiguilles --- */
    static void envoyerTramePositionAiguille(uint8_t index,
                                             uint8_t position,
                                             uint8_t etat);

    /* --- Booster : état + courant + tension + présence ---
     *
     * Trame envoyée (7 octets) :
     *   [0] SYNC       = 0xAA
     *   [1] OPCODE     = PROTO_07_BOOSTER
     *   [2] exsaAdresse (0 = H, 1 = AH)
     *   [3] etat       = état du booster :
     *                      0 = OFF
     *                      1 = OK
     *                      2 = FAULT
     *                      3 = OVERHEAT
     *   [4] courant    = courant mesuré (x10 mA)
     *   [5] tension    = tension mesurée (x100 mV)
     *   [6] present    = 1 si cet EXSA possède le booster
     *                    0 sinon
     */
    static void envoyerTrameBooster(uint8_t etat,
                                    uint8_t courant,
                                    uint8_t tension);

    /* --- RailCom : type + adresse ---
     *
     * Trame envoyée (6 octets) :
     *   [0] SYNC
     *   [1] OPCODE
     *   [2] exsaAdresse
     *   [3] type
     *   [4] adresse LSB
     *   [5] adresse MSB
     */
    static void envoyerTrameRailcom(uint8_t type,
                                    uint16_t adresse);

    /* --- Calibration Booster : seuilLibre + seuilOccupe ---
     *
     * Trame envoyée (7 octets) :
     *   [0] SYNC
     *   [1] OPCODE
     *   [2] exsaAdresse
     *   [3] LIBRE_LSB
     *   [4] LIBRE_MSB
     *   [5] OCCUPE_LSB
     *   [6] OCCUPE_MSB
     */
    static void envoyerTrameCalibBooster(uint16_t seuilLibre_mA,
                                         uint16_t seuilOccupe_mA);
};
