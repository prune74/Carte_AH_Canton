#pragma once
#include <Arduino.h>

/*
 * ============================================================
 *  EXSA_Pins.h — Mappage matériel officiel Discovery 2026
 * ------------------------------------------------------------
 *  Ce fichier définit **toutes les broches physiques** utilisées
 *  par l’EXSA :
 *
 *    - UART RS485 (SA ↔ EXSA)
 *    - Quadrature essieux
 *    - Charlieplexing (mât SNCF)
 *    - PCA9685 (LED canton, direction, servos)
 *    - MCP23017 (micro‑switchs, DIP, présence)
 *    - Booster (DRV8801 + ADC + RailCom)
 *    - CAN natif ESP32
 *
 *  ⚠️ Ce fichier est la **source de vérité unique** du hardware.
 *     Toute modification doit être synchronisée avec le PCB.
 * ============================================================
 */

/* ============================================================
   UART EXSA ↔ SA (RS485)
   ------------------------------------------------------------
   - RX/TX : UART matériel (Serial1)
   - DE/RE : contrôle half‑duplex du transceiver RS485
   ============================================================ */
static const int EXSA_UART_BAUDRATE = 115200;
static const gpio_num_t PIN_RS485_RX    = GPIO_NUM_25;
static const gpio_num_t PIN_RS485_TX    = GPIO_NUM_26;
static const gpio_num_t PIN_RS485_DE_RE = GPIO_NUM_12;

/* ============================================================
   Capteurs quadrature (A/B) — ESP32 (entrées rapides)
   ------------------------------------------------------------
   Utilisés pour le comptage essieux (EXSA_Quadrature).
   Entrées haute vitesse, sans pull‑up interne.
   ============================================================ */
static const gpio_num_t PIN_QUAD_A = GPIO_NUM_34;
static const gpio_num_t PIN_QUAD_B = GPIO_NUM_35;

/* ============================================================
   Charlieplexing (PWM + Hi-Z) — DOIT rester sur ESP32
   ------------------------------------------------------------
   Pilotage du mât SNCF (9 LED) via EXSA_Multiplexeur.
   Ces broches doivent supporter :
     - PWM (analogWrite)
     - haute impédance (INPUT)
   ============================================================ */
static const gpio_num_t PIN_MUX_P1 = GPIO_NUM_27;
static const gpio_num_t PIN_MUX_P2 = GPIO_NUM_13;
static const gpio_num_t PIN_MUX_P3 = GPIO_NUM_14;
static const gpio_num_t PIN_MUX_P4 = GPIO_NUM_18;

/* ============================================================
   PCA9685 — SORTIES (PWM)
   ------------------------------------------------------------
   Module 16 canaux utilisé pour :
     - Feux directionnels (4 LED)
     - LED canton (4 états)
     - Servos (3 aiguilles)
     - LED erreur Booster
   ============================================================ */

// Feux directionnels (barrette 1–4)
static const uint8_t PCA_DIR_1 = 0;
static const uint8_t PCA_DIR_2 = 1;
static const uint8_t PCA_DIR_3 = 2;
static const uint8_t PCA_DIR_4 = 3;

// LED erreur Booster (rouge)
static const uint8_t PCA_BOOSTER_ERROR = 7;

// LED canton (OCCUPÉ / LIBRE / MOUVEMENT / ERREUR)
static const uint8_t PCA_CANTON_OCCUPE     = 8;
static const uint8_t PCA_CANTON_LIBRE      = 9;
static const uint8_t PCA_CANTON_MOUVEMENT  = 10;
static const uint8_t PCA_CANTON_ERREUR     = 11;

// Servos (3 aiguilles)
static const uint8_t PCA_SERVO_1 = 12;
static const uint8_t PCA_SERVO_2 = 13;
static const uint8_t PCA_SERVO_3 = 14;

/* ============================================================
   MCP23017 — ENTRÉES UNIQUEMENT
   ------------------------------------------------------------
   Utilisé pour :
     - Micro‑switchs d’aiguilles
     - DIP H/AH + DIP Booster
     - Capteur présence
   ============================================================ */
static const uint8_t MCP23017_ADDR = 0x20;

// Micro-switchs aiguilles (DROIT / DEVIÉ)
static const uint8_t MCP_SW0_DROIT = 0;
static const uint8_t MCP_SW0_DEVIE = 1;
static const uint8_t MCP_SW1_DROIT = 2;
static const uint8_t MCP_SW1_DEVIE = 3;
static const uint8_t MCP_SW2_DROIT = 4;
static const uint8_t MCP_SW2_DEVIE = 5;

// DIP + capteur présence
static const uint8_t MCP_DIP_HAH     = 6;  // Horaire / Antihoraire
static const uint8_t MCP_DIP_BOOSTER = 7;  // Booster ON/OFF
static const uint8_t MCP_PRESENCE    = 8;  // Capteur IR / ILS

// Interruption MCP23017 (non utilisée dans EXSA 2026)
static const gpio_num_t PIN_MCP23017_INT = GPIO_NUM_17;

/* ============================================================
   Booster Discovery 2026 — ESP32 (temps réel)
   ------------------------------------------------------------
   Pilotage DRV8874 + mesures ADC + détection RailCom. :
     - nSLEEP : enable driver
     - PHASE  : sens DCC
     - FAULT  : retour erreur driver
     
   PWM DCC :
     - PIN_DCC_PWM : signal DCC haute fréquence
   ============================================================ */
static const gpio_num_t PIN_DRV_NSLEEP = GPIO_NUM_19;
static const gpio_num_t PIN_DRV_PHASE  = GPIO_NUM_21;
static const gpio_num_t PIN_DRV_FAULT  = GPIO_NUM_22;

static const gpio_num_t PIN_DCC_PWM    = GPIO_NUM_23;

/* ADC — Mesures analogiques Booster
   ------------------------------------------------------------
   - IPROPI  : courant (x10 mA)
   - VOLTAGE : tension (x100 mV)
   - RAILCOM : détection RailCom (fenêtre cutout)
   ============================================================ */
static const gpio_num_t PIN_ADC_IPROPI  = GPIO_NUM_32;
static const gpio_num_t PIN_ADC_VOLTAGE = GPIO_NUM_33;
static const gpio_num_t PIN_ADC_RAILCOM = GPIO_NUM_36;

/* ============================================================
   CAN Booster (natif ESP32)
   ------------------------------------------------------------
   Utilisé pour la topologie Discovery 2026.
   ============================================================ */
static const gpio_num_t PIN_CAN_RX = GPIO_NUM_4;
static const gpio_num_t PIN_CAN_TX = GPIO_NUM_5;
