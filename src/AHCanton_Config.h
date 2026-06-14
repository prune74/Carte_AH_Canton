#pragma once
#include <Arduino.h>
#include "AHCanton_Pins.h"          // Broches physiques

/*
 * ============================================================
 *  AHCanton_Config.h — Configuration globale AHCanton (Option A)
 * ------------------------------------------------------------
 *  Ce fichier regroupe toutes les constantes de configuration
 *  du firmware AHCanton. Il définit :
 *
 *    - options générales
 *    - temporisations canton
 *    - paramètres du multiplexeur Charlieplexing
 *    - intensités LED du mât SNCF
 *    - intensités LED directionnelles
 *    - seuils de sécurité du Booster Discovery 2026
 *    - paramètres analogiques DRV8874 (IPROPI)
 *
 *  ⚠️ Aucun aspect SNCF ici.
 *     Aucun protocole ici.
 *     Aucun bitfield ici.
 *
 *  Les aspects SNCF sont définis dans :
 *      → AHCanton_Signaux.h / AHCanton_Signaux.cpp
 * ============================================================
 */

/* ============================================================
 *  Options générales AHCanton
 * ============================================================ */
#define EXSA_DEBUG              false   // Debug série
#define EXSA_DEBOUNCE_MS        5       // Anti-rebond capteur présence
#define EXSA_MAX_ESSIEUX        200     // Limite sécurité comptage

/* ============================================================
 *  Type de signal (mât SNCF)
 * ------------------------------------------------------------
 *  false = signal principal (rouge)
 *  true  = signal de manœuvre (violet → carré violet)
 * ============================================================ */
#define EXSA_SIGNAL_EST_MANOEUVRE   false

/* ============================================================
 *  Temporisations EXSA_Canton
 * ------------------------------------------------------------
 *  - EXSA_CANTON_ANIM_STEP_MS : vitesse animation LED
 *  - EXSA_CANTON_MOUVEMENT_MS : durée LED "mouvement"
 * ============================================================ */
#define EXSA_CANTON_ANIM_STEP_MS    120
#define EXSA_CANTON_MOUVEMENT_MS    200

/* ============================================================
 *  Multiplexage Charlieplexing (mât SNCF)
 * ------------------------------------------------------------
 *  - EXSA_MUX_PERIOD_MS : fréquence de scan (1 kHz)
 *  - EXSA_MUX_BLINK_MS  : clignotement (2 Hz)
 *
 *  Le multiplexeur gère :
 *    - PWM
 *    - clignotement
 *    - haute impédance
 * ============================================================ */
#define EXSA_MUX_PERIOD_MS          1
#define EXSA_MUX_BLINK_MS           500

/* ============================================================
 *  Intensités par défaut des LED du mât SNCF (0–255)
 * ------------------------------------------------------------
 *  Valeurs PWM appliquées par EXSA_Multiplexeur.
 *  Chaque LED peut être modifiée dynamiquement.
 * ============================================================ */
#define INTENSITE_DEFAUT_LED_RALENTISSEMENT   220
#define INTENSITE_DEFAUT_LED_RAPPEL           200
#define INTENSITE_DEFAUT_LED_SEMAPHORE        255
#define INTENSITE_DEFAUT_LED_LIBRE            180
#define INTENSITE_DEFAUT_LED_AVERTISSEMENT    255
#define INTENSITE_DEFAUT_LED_OEILLETON        80
#define INTENSITE_DEFAUT_LED_MANOEUVRE        255
#define INTENSITE_DEFAUT_LED_CARRE            255
#define INTENSITE_DEFAUT_LED_CARRE_VIOLET     255

/* ============================================================
 *  Intensité des LED directionnelles (0–255)
 * ------------------------------------------------------------
 *  Utilisées par EXSA_LedDirection (PCA9685).
 * ============================================================ */
#define EXSA_DIR_LED0_INTENSITE   255
#define EXSA_DIR_LED1_INTENSITE   255
#define EXSA_DIR_LED2_INTENSITE   255
#define EXSA_DIR_LED3_INTENSITE   255

/* ============================================================
 *  Sécurité Booster Discovery 2026
 * ------------------------------------------------------------
 *  Seuils de protection voie :
 *
 *  - EXSA_BOOSTER_MAX_COURANT_mA
 *      Courant max avant coupure (court-circuit local)
 *
 *  - EXSA_BOOSTER_MIN_TENSION_mV
 *      Tension min avant coupure (voie OFF / alim faible)
 *
 *  - EXSA_BOOSTER_PHASE_TOLERANCE
 *      Tolérance inversion de phase DCC (0 = strict)
 *
 *  - EXSA_BOOSTER_ENABLE_GLOBAL_PROTECTION
 *      true  = coupe si un autre booster signale un défaut
 *      false = ignore les défauts des autres boosters
 *
 *  - EXSA_BOOSTER_ENABLE_GLOBAL_CUTOUT
 *      true  = coupe si cutout global actif
 *      false = ignore le cutout global
 * ============================================================ */
#define EXSA_BOOSTER_MAX_COURANT_mA        1400
#define EXSA_BOOSTER_MIN_TENSION_mV        12000
#define EXSA_BOOSTER_PHASE_TOLERANCE       0
#define EXSA_BOOSTER_ENABLE_GLOBAL_PROTECTION  true
#define EXSA_BOOSTER_ENABLE_GLOBAL_CUTOUT      true

/* ============================================================
 *  Mesure courant via DRV8874 (IPROPI)
 * ------------------------------------------------------------
 *  Formule :
 *      I = V_IPROPI / (R_IPROPI * A_IPROPI)
 *
 *  Avec :
 *      R_IPROPI = 3,6 kΩ
 *      A_IPROPI = 455 µA/A
 *
 *  Permet une mesure précise du courant voie.
 * ============================================================ */
#define EXSA_IPROPI_R_OHMS          3600.0f
#define EXSA_IPROPI_GAIN_A_PER_A    0.000455f

/* ============================================================
 *  Mesure tension voie DCC (pont diviseur)
 * ------------------------------------------------------------
 *  Pont diviseur :
 *      R1 = 68 kΩ (haut)
 *      R2 = 10 kΩ (bas)
 *
 *  Formule :
 *      Vrail = Vmes * (R1 + R2) / R2
 * ============================================================ */
#define EXSA_ADC_VOLTAGE_R1_OHMS     68000.0f
#define EXSA_ADC_VOLTAGE_R2_OHMS     10000.0f

// Tension de référence ADC ESP32
#define EXSA_ADC_VREF                3.3f

// Facteur de conversion (pré-calculé)
#define EXSA_ADC_VOLTAGE_FACTOR      ((EXSA_ADC_VOLTAGE_R1_OHMS + EXSA_ADC_VOLTAGE_R2_OHMS) / EXSA_ADC_VOLTAGE_R2_OHMS)


/* ============================================================
 *  État global du Booster Discovery 2026
 * ------------------------------------------------------------
 *  Utilisé dans EXSA_BoosterCore pour :
 *    - télémétrie
 *    - sécurité
 *    - trames 0x07 vers SA
 * ============================================================ */
enum ExsaBoosterEtat : uint8_t
{
    BOOSTER_OFF = 0,
    BOOSTER_OK = 1,
    BOOSTER_COURT_CIRCUIT = 2,
    BOOSTER_SOUS_TENSION = 3,
    BOOSTER_SURCHAUFFE = 4,
    BOOSTER_ERREUR = 5
};

/* ============================================================
 *  Détection d’occupation par courant (Booster Discovery 2026)
 * ------------------------------------------------------------
 *  L’occupation du canton est déterminée par la consommation
 *  de courant mesurée via IPROPI (DRV8874).
 *
 *  Hystérésis :
 *    - Si LIBRE  → devient OCCUPÉ si courant > EXSA_SEUIL_OCCUPE_mA
 *    - Si OCCUPÉ → reste OCCUPÉ tant que courant > EXSA_SEUIL_LIBRE_mA
 *
 *  Ces valeurs dépendent du matériel (loco, éclairage, bruit ADC).
 * ============================================================ */
#define EXSA_SEUIL_OCCUPE_mA    15     // au-dessus → OCCUPÉ
#define EXSA_SEUIL_LIBRE_mA     8      // en-dessous → LIBRE

/*
 * ============================================================
 *  Aucun aspect ici !
 * ------------------------------------------------------------
 *  Les aspects SNCF sont définis dans :
 *      → EXSA_Signaux.h (enum ExsaAspect)
 *
 *  Le protocole SA → EXSA utilise un octet (enum simple).
 * ============================================================
 */
