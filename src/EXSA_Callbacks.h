#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXSA_Callbacks.h — Callbacks SA → EXSA
 *  Version optimisée (header minimal)
 * ============================================================
 */

namespace EXSA_Callbacks
{
    /* --- Topologie / Configuration --- */
    void onTopologie(uint8_t *data, uint8_t len) noexcept;
    void onConfigSignaux(uint8_t *data, uint8_t len) noexcept;

    /* --- Aspects SNCF --- */
    void onAspectHoraire(uint8_t aspect) noexcept;
    void onAspectAntiHoraire(uint8_t aspect) noexcept;

    /* --- Directions LED --- */
    void onDirectionHoraire(uint8_t code) noexcept;
    void onDirectionAntiHoraire(uint8_t code) noexcept;

    /* --- Occupation voisins --- */
    void onOccupationVoisins(uint8_t value) noexcept;

    /* --- Servos --- */
    void onServoMove(uint8_t servoIndex, uint8_t direction) noexcept;
    void onServoConfig(uint8_t servoIndex,
                       uint16_t posDroit,
                       uint16_t posDevie,
                       uint16_t speed) noexcept;
    void onServoTest(uint8_t servoIndex) noexcept;

    /* --- 🟩 Calibration Booster (F3) --- */
    void onRecalibrationBooster() noexcept;

    /* --- 🟩 Application des seuils envoyés par le SA (F4) --- */
    void onSetSeuils(uint16_t libre, uint16_t occupe) noexcept;
    
    /* --- 🟩 Booster ON/OFF (F5) --- */
    void onBoosterPower(uint8_t power) noexcept;
}
