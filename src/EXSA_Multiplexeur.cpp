/*
 * Module : EXSA_Multiplexeur
 * Rôle   : Pilotage du mât SNCF via Charlieplexing (4 GPIO → 9 LED).
 *
 * Fonctionnement général :
 *   - Chaque LED du mât est définie par une paire (anode, cathode)
 *     parmi les 4 lignes P1..P4.
 *   - Pour allumer une LED :
 *        • Anode  = PWM (intensité)
 *        • Cathode = LOW
 *        • Les 2 autres lignes = haute impédance (INPUT)
 *
 *   - Le module gère :
 *        • l’état ON/OFF de chaque LED
 *        • l’intensité individuelle (0–255)
 *        • le clignotement (phase ON/OFF)
 *        • le scan Charlieplexing (1 LED active à la fois)
 *
 * Flux entrants :
 *   - réglerLed() / réglerIntensité() depuis EXSA_Signaux
 *   - mettreAJour() appelé périodiquement par EXSA_Runtime
 *
 * Flux sortants :
 *   - Commandes GPIO (INPUT / OUTPUT / PWM)
 *
 * Contraintes temps réel :
 *   - Aucun malloc
 *   - Scan rapide (EXSA_MUX_PERIOD_MS)
 *   - Clignotement indépendant (EXSA_MUX_BLINK_MS)
 *   - Une seule LED active à la fois → Charlieplexing strict
 *
 * Notes :
 *   - Le PWM est généré par analogWrite() (ESP32)
 *   - Le multiplexeur doit être mis à jour très régulièrement
 *   - Les intensités par défaut viennent de EXSA_Config.h
 */

#include "EXSA_Multiplexeur.h"
#include "EXSA_Config.h"

#include <Arduino.h>

/* ------------------------------------------------------------
 * Tables de mapping LED ↔ anode/cathode
 * ------------------------------------------------------------
 * Chaque LED du mât est définie par une paire (anode, cathode)
 * correspondant à deux des 4 GPIO du Charlieplexing.
 *
 * Exemple :
 *   LED_RALENTISSEMENT = ANODE=0 (P1), CATHODE=1 (P2)
 *   → pour l’allumer : P1 = PWM, P2 = LOW, les autres = INPUT.
 *
 * Ces tables définissent la topologie physique du mât.
 * ------------------------------------------------------------ */

static const uint8_t ANODE[LED_MAX] = {
    0, // LED_RALENTISSEMENT  P1->P2
    1, // LED_RAPPEL          P2->P1
    1, // LED_SEMAPHORE       P2->P3
    2, // LED_LIBRE           P3->P2
    1, // LED_AVERTISSEMENT   P2->P4
    3, // LED_OEILLETON       P4->P2
    2, // LED_MANOEUVRE       P3->P4
    3, // LED_CARRE2          P4->P3
    0  // LED_CARRE_VIOLET    P1->P4
};

static const uint8_t CATHODE[LED_MAX] = {
    1, // P2
    0, // P1
    2, // P3
    1, // P2
    3, // P4
    1, // P2
    3, // P4
    2, // P3
    3  // P4
};

/* ------------------------------------------------------------
 * Intensité par défaut de chaque LED
 * ------------------------------------------------------------
 * Valeurs définies dans EXSA_Config.h.
 * Elles peuvent être modifiées dynamiquement via réglerLed().
 * ------------------------------------------------------------ */
static const uint8_t INTENSITE_DEFAUT[LED_MAX] = {
    [LED_RALENTISSEMENT]   = INTENSITE_DEFAUT_LED_RALENTISSEMENT,
    [LED_RAPPEL]           = INTENSITE_DEFAUT_LED_RAPPEL,
    [LED_SEMAPHORE]        = INTENSITE_DEFAUT_LED_SEMAPHORE,
    [LED_LIBRE]            = INTENSITE_DEFAUT_LED_LIBRE,
    [LED_AVERTISSEMENT]    = INTENSITE_DEFAUT_LED_AVERTISSEMENT,
    [LED_OEILLETON]        = INTENSITE_DEFAUT_LED_OEILLETON,
    [LED_MANOEUVRE]        = INTENSITE_DEFAUT_LED_MANOEUVRE,
    [LED_CARRE]            = INTENSITE_DEFAUT_LED_CARRE,
    [LED_CARRE_VIOLET]     = INTENSITE_DEFAUT_LED_CARRE_VIOLET
};

/* ------------------------------------------------------------
 * Constructeur
 * ------------------------------------------------------------
 * Initialise :
 *   - les 4 lignes P1..P4 en haute impédance
 *   - l’état interne de chaque LED (OFF, intensité, clignote)
 *   - les timers de scan et clignotement
 * ------------------------------------------------------------ */
EXSA_Multiplexeur::EXSA_Multiplexeur(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4)
{
    P[0] = p1;
    P[1] = p2;
    P[2] = p3;
    P[3] = p4;

    // Toutes les lignes en haute impédance
    for (int i = 0; i < 4; i++)
        pinMode(P[i], INPUT);

    // État initial des LED
    for (int i = 0; i < LED_MAX; i++) {
        etats[i].allumer = false;
        etats[i].intensite = INTENSITE_DEFAUT[i];
        etats[i].clignote = false;
        etats[i].etatClignote = true; // phase ON
    }

    dernierCligno = 0;
    dernierScan = 0;
    indexScan = 0;
}

/* ------------------------------------------------------------
 * réglerLed()
 * ------------------------------------------------------------
 * Définit :
 *   - allumer / éteindre
 *   - intensité (0–255)
 *   - clignotement ON/OFF
 * ------------------------------------------------------------ */
void EXSA_Multiplexeur::reglerLed(ExsaLedId id, bool allumer, uint8_t intensite, bool clignote)
{
    etats[id].allumer = allumer;
    etats[id].intensite = intensite;
    etats[id].clignote = clignote;
}

/* ------------------------------------------------------------
 * réglerIntensité()
 * ------------------------------------------------------------
 * Change uniquement l’intensité d’une LED.
 * ------------------------------------------------------------ */
void EXSA_Multiplexeur::reglerIntensite(ExsaLedId id, uint8_t intensite)
{
    etats[id].intensite = intensite;
}

/* ------------------------------------------------------------
 * configLigne()
 * ------------------------------------------------------------
 * Configure une ligne P1..P4 :
 *   - INPUT  → haute impédance
 *   - OUTPUT → LOW ou HIGH
 * ------------------------------------------------------------ */
void EXSA_Multiplexeur::configLigne(uint8_t pin, int mode, int valeur)
{
    pinMode(pin, mode);
    if (mode == OUTPUT)
        digitalWrite(pin, valeur);
}

/* ------------------------------------------------------------
 * appliquerLed()
 * ------------------------------------------------------------
 * Active une LED selon son index :
 *   1) Toutes les lignes → INPUT (haute impédance)
 *   2) Si LED OFF → rien
 *   3) Si clignotement OFF → rien
 *   4) Cathode = LOW
 *   5) Anode = PWM (intensité)
 *
 * Une seule LED est active à la fois → Charlieplexing.
 * ------------------------------------------------------------ */
void EXSA_Multiplexeur::appliquerLed(uint8_t index)
{
    // 1) Tout en haute impédance
    for (int i = 0; i < 4; i++)
        configLigne(P[i], INPUT);

    // 2) LED éteinte
    if (!etats[index].allumer)
        return;

    // 3) LED clignotante mais phase OFF
    if (etats[index].clignote && !etats[index].etatClignote)
        return;

    // 4) Récupération anode/cathode
    uint8_t a = ANODE[index];
    uint8_t c = CATHODE[index];

    // 5) Cathode = LOW
    configLigne(P[c], OUTPUT, LOW);

    // 6) Anode = PWM
    analogWrite(P[a], etats[index].intensite);
}

/* ------------------------------------------------------------
 * mettreAJour()
 * ------------------------------------------------------------
 * Appelé régulièrement (EXSA_Runtime).
 *
 * 1) Mise à jour du clignotement :
 *      - toutes les EXSA_MUX_BLINK_MS
 *      - inverse la phase ON/OFF des LED clignotantes
 *
 * 2) Scan Charlieplexing :
 *      - toutes les EXSA_MUX_PERIOD_MS
 *      - active une LED à la fois
 *      - avance indexScan circulairement
 * ------------------------------------------------------------ */
void EXSA_Multiplexeur::mettreAJour()
{
    uint32_t maintenant = millis();

    // 1) Clignotement
    if (maintenant - dernierCligno >= EXSA_MUX_BLINK_MS) {
        dernierCligno = maintenant;
        for (int i = 0; i < LED_MAX; i++)
            if (etats[i].clignote)
                etats[i].etatClignote = !etats[i].etatClignote;
    }

    // 2) Scan Charlieplexing
    if (maintenant - dernierScan >= EXSA_MUX_PERIOD_MS) {
        dernierScan = maintenant;

        appliquerLed(indexScan);

        indexScan = (indexScan + 1) % LED_MAX;
    }
}
