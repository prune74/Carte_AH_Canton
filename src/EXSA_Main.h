#pragma once
#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>

#include "EXSA_Signaux.h"
#include "EXSA_LedDirection.h"

/*
 * ============================================================
 *  Interface EXSA_Main — Version refactorisée 2026
 * ============================================================
 */

namespace EXSA_Main
{
    void begin() noexcept;
    void loop() noexcept;
}

/* ============================================================
   Variables / objets globaux partagés
   ============================================================ */

// PCA9685 global
extern Adafruit_PWMServoDriver g_pca9685;

// Adressage / mode EXSA
extern uint8_t exsaAdresse;     // 0 = H, 1 = AH
extern bool    exsaIsHoraire;   // true = H, false = AH
extern bool    exsaHasBooster;  // DIP Booster

// Instances globales EXSA
extern EXSA_Signaux      signaux;
extern EXSA_LedDirection direction;
