/*
 * Module : EXSA_Callbacks
 * Rôle   : Point d’entrée des commandes SA → EXSA.
 */

#include "EXSA_Callbacks.h"
#include "EXSA_Main.h"
#include "EXSA_Signaux.h"
#include "EXSA_LedDirection.h"
#include "EXSA_Canton.h"
#include "EXSA_Servo.h"
#include "EXSA_Switches.h"
#include "EXSA_Calibration.h"
#include "EXSA_BoosterCore.h"

#include <Arduino.h>

extern EXSA_Signaux signaux;
extern EXSA_LedDirection direction;

/* ============================================================
   Topologie (E4)
============================================================ */
void EXSA_Callbacks::onTopologie(uint8_t *data, uint8_t len) noexcept
{
#if EXSA_DEBUG
    Serial.println("[EXSA] Topologie reçue");
#endif
}

/* ============================================================
   Configuration signaux (E5)
============================================================ */
void EXSA_Callbacks::onConfigSignaux(uint8_t *data, uint8_t len) noexcept
{
#if EXSA_DEBUG
    Serial.println("[EXSA] Config signaux OK");
#endif
}

/* ============================================================
   Aspects SNCF (E6 / E7)
============================================================ */
void EXSA_Callbacks::onAspectHoraire(uint8_t aspect) noexcept
{
    (void)signaux.setAspect((ExsaAspect)aspect);
}

void EXSA_Callbacks::onAspectAntiHoraire(uint8_t aspect) noexcept
{
    (void)signaux.setAspect((ExsaAspect)aspect);
}

/* ============================================================
   Direction (E8 / E9)
============================================================ */
void EXSA_Callbacks::onDirectionHoraire(uint8_t code) noexcept
{
    (void)direction.setDirection(code);
}

void EXSA_Callbacks::onDirectionAntiHoraire(uint8_t code) noexcept
{
    (void)direction.setDirection(code);
}

/* ============================================================
   Occupation voisins (EA)
============================================================ */
void EXSA_Callbacks::onOccupationVoisins(uint8_t value) noexcept
{
    (void)EXSA_Canton::setVoisins(value);
}

/* ============================================================
   Servos (F0 / F1 / F2)
============================================================ */
void EXSA_Callbacks::onServoMove(uint8_t servoIndex, uint8_t dir) noexcept
{
    (void)EXSA_Servo::move(servoIndex, dir);
    (void)EXSA_Switches::notifierMouvementDemarre(servoIndex);
}

void EXSA_Callbacks::onServoConfig(uint8_t servoIndex,
                                   uint16_t posDroit,
                                   uint16_t posDevie,
                                   uint16_t speed) noexcept
{
    (void)EXSA_Servo::configure(servoIndex, posDroit, posDevie, speed);
}

void EXSA_Callbacks::onServoTest(uint8_t servoIndex) noexcept
{
    (void)EXSA_Servo::test(servoIndex);
}

/* ============================================================
   🟩 Recalibration Booster (F3)
============================================================ */
void EXSA_Callbacks::onRecalibrationBooster() noexcept
{
#if EXSA_DEBUG
    Serial.println("[EXSA] Recalibration Booster demandée");
#endif

    EXSA_Calibration::start();
}

/* ============================================================
   🟩 Application des seuils envoyés par le SA (F4)
   ------------------------------------------------------------
   Le SA renvoie les seuils stockés dans settings.json.
   → EXSA_Calibration::setSeuils()
   ------------------------------------------------------------
   - Non bloquant
   - Idempotent
   - Aucun retour
============================================================ */
void EXSA_Callbacks::onSetSeuils(uint16_t libre, uint16_t occupe) noexcept
{
#if EXSA_DEBUG
    Serial.printf("[EXSA] SET SEUILS : libre=%u occupe=%u\n", libre, occupe);
#endif

    EXSA_Calibration::setSeuils(libre, occupe);
}

/* ============================================================
   🟩 Booster ON/OFF (F5)
   ------------------------------------------------------------
   - power = 0 → OFF
   - power = 1 → ON
    ------------------------------------------------------------
    - Non bloquant              
    - Idempotent
    - Aucun retour
============================================================ */
void EXSA_Callbacks::onBoosterPower(uint8_t power) noexcept
{   
#if EXSA_DEBUG
    Serial.printf("[EXSA] BOOSTER POWER : %s\n", power ? "ON" : "OFF");
#endif

    EXSA_BoosterCore::setEnabled(power != 0);
}