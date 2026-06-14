/*
 * Module : EXSA_Calibration
 * Rôle   : Auto‑calibration NON BLOQUANTE du courant à vide.
 *
 * Fonctionnement général :
 *   - start()   : reset + début calibration
 *   - process() : 1 échantillon par ms (appelé dans BoosterCore)
 *   - isDone()  : seuils prêts
 *
 * Avantages :
 *   - aucune pause 1 seconde
 *   - 100 % temps réel
 *   - parfaitement compatible FreeRTOS
 */

#include "EXSA_Calibration.h"
#include "EXSA_Booster.h"
#include "EXSA_Config.h"

#include <Arduino.h>
#include <math.h>

/* ============================================================
 * Variables internes
 * ============================================================ */
static uint16_t s_seuilLibre_mA  = EXSA_SEUIL_LIBRE_mA;
static uint16_t s_seuilOccupe_mA = EXSA_SEUIL_OCCUPE_mA;

static bool     s_running   = false;
static bool     s_done      = false;
static uint16_t s_index     = 0;

static const uint16_t CALIB_N = 1000; // 1 seconde à 1 kHz

static uint32_t s_somme       = 0;
static uint32_t s_sommeCarres = 0;

/* ============================================================
 * init() — seuils par défaut
 * ============================================================ */
void EXSA_Calibration::init()
{
    s_seuilLibre_mA  = EXSA_SEUIL_LIBRE_mA;
    s_seuilOccupe_mA = EXSA_SEUIL_OCCUPE_mA;
    s_running = false;
    s_done    = false;
}

/* ============================================================
 * setSeuils() — seuils imposés (ex : depuis SA)
 * ============================================================ */
void EXSA_Calibration::setSeuils(uint16_t libre, uint16_t occupe)
{
    s_seuilLibre_mA  = libre;
    s_seuilOccupe_mA = occupe;
}

/* ============================================================
 * Accesseurs
 * ============================================================ */
uint16_t EXSA_Calibration::getSeuilLibre()  { return s_seuilLibre_mA; }
uint16_t EXSA_Calibration::getSeuilOccupe() { return s_seuilOccupe_mA; }

bool EXSA_Calibration::isRunning() { return s_running; }
bool EXSA_Calibration::isDone()    { return s_done; }

/* ============================================================
 * start() — lance une calibration non bloquante
 * ============================================================ */
void EXSA_Calibration::start()
{
    s_running   = true;
    s_done      = false;
    s_index     = 0;
    s_somme     = 0;
    s_sommeCarres = 0;
}

/* ============================================================
 * process() — appelé toutes les 1 ms
 * ============================================================ */
void EXSA_Calibration::process()
{
    if (!s_running)
        return;

    // 1 échantillon par ms
    uint16_t i_mA = EXSA_Booster::readCurrent_mA();
    s_somme       += i_mA;
    s_sommeCarres += (uint32_t)i_mA * i_mA;
    s_index++;

    // Fin de calibration
    if (s_index >= CALIB_N)
    {
        float moyenne      = float(s_somme) / CALIB_N;
        float moyenneCarre = float(s_sommeCarres) / CALIB_N;
        float variance     = max(0.0f, moyenneCarre - moyenne * moyenne);
        float bruit        = sqrtf(variance);

        s_seuilLibre_mA  = uint16_t(moyenne + 3.0f * bruit);
        s_seuilOccupe_mA = s_seuilLibre_mA + 5;

        s_running = false;
        s_done    = true;

#if EXSA_DEBUG
        Serial.printf("[CALIB] moyenne=%.2f bruit=%.2f\n", moyenne, bruit);
        Serial.printf("[CALIB] seuilLibre=%u seuilOccupe=%u\n",
                      s_seuilLibre_mA, s_seuilOccupe_mA);
#endif
    }
}
