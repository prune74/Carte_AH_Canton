#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  DISCOVERY 2026 — PROTOCOLE OFFICIEL
 * ------------------------------------------------------------
 *  Fichier unique partagé entre :
 *    • DiscoveryMaster (Master)
 *    • Satellites Autonomes (SA)
 *    • Extensions Satellites (EXSA)
 *
 *  Contient :
 *    • Aspects SNCF (enum)
 *    • Protocole SA ↔ EXSA (PROTO_xx)
 *    • Protocole Master ↔ SA (CMD_xx)
 *
 *  Objectif :
 *    → Garantir une cohérence totale du protocole Discovery 2026
 *    → Éviter les divergences entre firmwares
 *    → Faciliter la maintenance et l’évolution
 * ============================================================
 */


/* ============================================================
 *  🟦 ENUM DES ASPECTS SNCF — 1 octet
 * ============================================================
 */

enum ExsaAspect : uint8_t {
    ASPECT_CARRE = 0,            // 🔴 Arrêt absolu
    ASPECT_SEMAPHORE,            // 🔴 Arrêt
    ASPECT_AVERTISSEMENT,        // 🟡 Ralentir
    ASPECT_RALENTISSEMENT_30,    // 🟡⚠️ 30 km/h
    ASPECT_RALENTISSEMENT_60,    // 🟡⚠️ 60 km/h
    ASPECT_RAPPEL_30,            // 🟡🔁 Rappel 30
    ASPECT_RAPPEL_60,            // 🟡🔁 Rappel 60
    ASPECT_VOIE_LIBRE,           // 🟢 Voie libre
    ASPECT_MANOEUVRE,            // 🔵 Manoeuvre
    ASPECT_MASQUE,               // ⚫ Masqué
    ASPECT_DEFAUT                // ⚠️ Défaut
};


/* ============================================================
 *  🟧 PROTOCOLE SA ↔ EXSA — Format série 0xAA / OPCODE
 * ============================================================
 */

 /*
 * Format général des trames série SA ↔ EXSA (RS485)
 * ------------------------------------------------
 *
 *  SYNC  : PROTO_SYNC_BYTE (0xAA)
 *  OPCODE: PROTO_xx ci‑dessous
 *  DATA  : 0 à N octets selon l’OPCODE
 *
 *  Convention Discovery 2026 :
 *
 *  • SA → EXSA
 *      - PROTO_PING :
 *          [SYNC][PING][index_exsa]
 *          index_exsa = 0 (Horaire) ou 1 (AntiHoraire)
 *      - les autres opcodes gardent leur format existant
 *
 *  • EXSA → SA
 *      - Toutes les trames commencent par :
 *          [SYNC][OPCODE][index_exsa][...]
 *        où index_exsa = 0 (Horaire) ou 1 (AntiHoraire)
 *
 *      - Exemples :
 *          PROTO_PONG :
 *              [SYNC][PONG][index_exsa]
 *
 *          PROTO_04_OCCUPATION :
 *              [SYNC][0x04][index_exsa][valeur]
 *
 *          PROTO_07_BOOSTER :
 *              [SYNC][0x07][index_exsa][etat][courant][tension][present]
 */

#define PROTO_SYNC_BYTE 0xAA    // Octet de synchronisation

/* --- Supervision --- */
#define PROTO_PING                      0x32    // SA → EXSA : ping
#define PROTO_PONG                      0x33    // EXSA → SA : pong

/* --- Topologie / Configuration --- */
#define PROTO_E4_TOPOLOGIE_CAN          0xE4    // Topologie CAN
#define PROTO_E5_CONFIG_SIGNAUX         0xE5    // Configuration signaux

/* --- Aspects SNCF --- */
#define PROTO_E6_ASPECT_HORAIRE         0xE6    // Aspect côté H
#define PROTO_E7_ASPECT_ANTIHORAIRE     0xE7    // Aspect côté AH

/* --- Feux directionnels --- */
#define PROTO_E8_DIRECTION_HORAIRE      0xE8
#define PROTO_E9_DIRECTION_ANTIHORAIRE  0xE9

/* --- Occupation voisins --- */
#define PROTO_EA_OCCUPATION_VOISINS     0xEA

/* --- Servos --- */
#define PROTO_F0_SERVO_MOVE             0xF0
#define PROTO_F1_SERVO_CONFIG           0xF1
#define PROTO_F2_SERVO_TEST             0xF2

/* --- Booster (SA → EXSA) --- */
#define PROTO_F3_RECALIBRER_BOOSTER     0xF3    // Recalibration automatique
#define PROTO_F4_SET_SEUILS             0xF4    // Seuils calibrés (4 octets)
#define PROTO_F5_BOOSTER_POWER          0xF5    // Booster ON/OFF (0=OFF, 1=ON)

/* ============================================================
 *  🟩 EXSA → SA : Informations remontées
 * ============================================================
 */

#define PROTO_03_PONCTUEL               0x03    // Retour ponctuel H/AH
#define PROTO_04_OCCUPATION             0x04    // Occupation canton
#define PROTO_05_DELTA_AXE              0x05    // Variation aiguille
#define PROTO_06_POSITION_AIGUILLE      0x06    // Position réelle
#define PROTO_07_BOOSTER                0x07    // État booster
#define PROTO_08_RAILCOM_ADRESSE        0x08    // Adresse RailCom
#define PROTO_09_CALIB_BOOSTER          0x09    // Seuils calibrés


/* ============================================================
 *  🟪 Codes associés EXSA → SA
 * ============================================================
 */

/* --- PONCTUEL --- */
#define PROTO_PONCT_H_ACTIVE            0x10
#define PROTO_PONCT_H_INACTIVE          0x11
#define PROTO_PONCT_AH_ACTIVE           0x12
#define PROTO_PONCT_AH_INACTIVE         0x13

/* --- Occupation --- */
#define PROTO_OCC_ACTIVE                0x30
#define PROTO_OCC_LIBRE                 0x31

/* --- Delta axe --- */
#define PROTO_DELTA_PLUS_UN             0x01
#define PROTO_DELTA_MOINS_UN            0xFF

/* --- Position aiguille --- */
#define PROTO_POS_DROIT                 0x00
#define PROTO_POS_DEVIE                 0x01
#define PROTO_POS_INDET                 0x02
#define PROTO_POS_INCOHERENT            0x03

/* --- États sécurité --- */
#define PROTO_ETAT_OK                   0x00
#define PROTO_ETAT_BLOQUE               0x01
#define PROTO_ETAT_ERREUR               0x02


/* ============================================================
 *  🟦 PROTOCOLE MASTER ↔ SA — Bus CAN 29 bits
 * ============================================================
 *
 *  Ces commandes sont envoyées par :
 *    • DiscoveryMaster (Master)
 *    • Satellites (SA / EXSA)
 *
 *  Elles utilisent le format CAN étendu Märklin 29 bits.
 * ============================================================
 */

/* --- Master → Satellites --- */
#define CMD_WIFI_ON_OFF                 0xBD    // Active/désactive WiFi
#define CMD_DISCOVERY_ON_OFF            0xBE    // Active/désactive Discovery
#define CMD_SAVE_ALL                    0xBF    // Sauvegarde globale
#define CMD_RESTART_ALL                 0xBC    // Redémarrage global
#define CMD_SET_PROFILE                 0x20    // Profil voie (0=N, 1=HO)

/* --- Satellites → Master --- */
#define CMD_SAT_HEARTBEAT               0xB0    // Heartbeat
#define CMD_SAT_TEST_BUS                0xB2    // Test bus CAN
#define CMD_SAT_TEST_BUS_REPLY          0xB3    // Réponse test bus
#define CMD_SAT_REQUEST_ID              0xB4    // Demande ID
#define CMD_SAT_REQUEST_ID_REPLY        0xB5    // Réponse ID


/* ============================================================
 *  FIN — Discovery_Protocol.h
 * ============================================================
 */
