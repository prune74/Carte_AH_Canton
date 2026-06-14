/*
 * Module : EXSA_Servo
 * Rôle   : Pilotage avancé des servos d’aiguillage Discovery 2026.
 *
 * Fonctionnement général :
 *   - 3 servos maximum (PCA9685)
 *   - Position droite / déviée en microsecondes (converties en PWM)
 *   - Interpolation progressive (vitesse configurable)
 *   - Modes avancés :
 *        • Servo OFF : PWM = 0 quand position atteinte
 *        • Soft‑Start : accélération progressive du mouvement
 *        • Deadband : zone morte anti‑oscillation
 *
 * Flux entrants :
 *   - move()      : commande SA F0
 *   - configure() : commande SA F1
 *   - test()      : commande SA F2
 *
 * Flux sortants :
 *   - PWM vers PCA9685
 *   - Accesseurs pour EXSA_Switches (sécurité anti‑blocage)
 *
 * Contraintes temps réel :
 *   - Aucun malloc
 *   - update() doit être très court (appelé ~1 kHz)
 *   - Interpolation déterministe
 *
 * Notes :
 *   - Le module ne gère pas la sécurité : EXSA_Switches surveille
 *     les mouvements anormaux ou bloqués.
 */

#include "EXSA_Servo.h"
#include "EXSA_Pins.h"
#include "EXSA_Config.h"
#include "EXSA_Switches.h"

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

/*
 * ============================================================
 *  EXSA_Servo.cpp — Version améliorée 2026
 * ------------------------------------------------------------
 *  Ajouts :
 *    - Mode Servo OFF (PWM = 0 quand position atteinte)
 *    - Soft‑Start (accélération progressive)
 *    - Deadband (zone morte anti‑oscillation)
 * ============================================================
 */

extern Adafruit_PWMServoDriver g_pca9685;

namespace
{
    // Nombre de servos gérés
    constexpr uint8_t SERVO_COUNT = 3;

    // Canaux PCA9685 associés aux servos
    constexpr uint8_t SERVO_CHANNEL[SERVO_COUNT] = {
        PCA_SERVO_1,
        PCA_SERVO_2,
        PCA_SERVO_3
    };

    // Plage microsecondes → PWM
    constexpr uint16_t SERVO_US_MIN   = 800;
    constexpr uint16_t SERVO_US_MAX   = 2400;
    constexpr uint16_t SERVO_US_RANGE = SERVO_US_MAX - SERVO_US_MIN;

    // Modes avancés
    bool servoOffEnabled   = true;
    bool softStartEnabled  = true;
    uint16_t deadband      = 3;

    // Conversion µs → PWM PCA9685 (0–4095)
    inline uint16_t usToPwm(uint16_t us)
    {
        if (us < SERVO_US_MIN) us = SERVO_US_MIN;
        else if (us > SERVO_US_MAX) us = SERVO_US_MAX;

        return (uint32_t)(us - SERVO_US_MIN) * 4095u / SERVO_US_RANGE;
    }

    // Structure interne d’un servo
    struct ServoData {
        uint16_t posDroit_pwm;
        uint16_t posDevie_pwm;
        uint16_t speed_pwm;
        uint16_t current_pwm;
        uint16_t target_pwm;

        uint16_t softStartCounter = 1;
    };

    ServoData servos[SERVO_COUNT];

    inline bool indexValide(uint8_t index)
    {
        return index < SERVO_COUNT;
    }

    inline void appliquerPWM(uint8_t index)
    {
        g_pca9685.setPWM(SERVO_CHANNEL[index], 0, servos[index].current_pwm);
    }
}

/* ============================================================
 * begin()
 * ------------------------------------------------------------
 * Initialise les servos :
 *   - positions par défaut = 1500 µs
 *   - vitesse par défaut
 *   - PWM appliqué immédiatement
 * ============================================================ */
void EXSA_Servo::begin() noexcept
{
    for (uint8_t i = 0; i < SERVO_COUNT; ++i)
    {
        servos[i].posDroit_pwm = usToPwm(1500);
        servos[i].posDevie_pwm = usToPwm(1500);
        servos[i].speed_pwm    = 10;

        servos[i].current_pwm  = servos[i].posDroit_pwm;
        servos[i].target_pwm   = servos[i].posDroit_pwm;

        servos[i].softStartCounter = 1;

        appliquerPWM(i);
    }
}

/* ============================================================
 * update()
 * ------------------------------------------------------------
 * Interpolation progressive + modes avancés :
 *   - Deadband : si proche de la cible → snap + servo OFF
 *   - Soft‑Start : accélération progressive
 *   - Servo OFF : PWM = 0 quand position atteinte
 *
 * Appelé en continu (1 kHz).
 * ============================================================ */
void EXSA_Servo::update() noexcept
{
    for (uint8_t i = 0; i < SERVO_COUNT; ++i)
    {
        uint16_t cur = servos[i].current_pwm;
        uint16_t tgt = servos[i].target_pwm;

        // Zone morte : position atteinte
        if (abs((int)cur - (int)tgt) <= deadband)
        {
            servos[i].current_pwm = tgt;

            if (servoOffEnabled)
                g_pca9685.setPWM(SERVO_CHANNEL[i], 0, 0);
            else
                appliquerPWM(i);

            continue;
        }

        // Vitesse de base
        uint16_t step = servos[i].speed_pwm;

        // Soft‑Start : accélération progressive
        if (softStartEnabled)
        {
            if (servos[i].softStartCounter < step)
                servos[i].softStartCounter++;

            step = servos[i].softStartCounter;
        }

        // Interpolation vers la cible
        if (cur < tgt)
        {
            uint16_t delta = tgt - cur;
            servos[i].current_pwm = (delta <= step) ? tgt : cur + step;
        }
        else
        {
            uint16_t delta = cur - tgt;
            servos[i].current_pwm = (delta <= step) ? tgt : cur - step;
        }

        appliquerPWM(i);
    }
}

/* ============================================================
 * F0 — move(index, direction)
 * ------------------------------------------------------------
 * direction = 0 → position droite
 * direction = 1 → position déviée
 *
 * Reset du soft‑start à chaque mouvement.
 * ============================================================ */
void EXSA_Servo::move(uint8_t index, uint8_t direction) noexcept
{
    if (!indexValide(index))
        return;

    servos[index].target_pwm =
        (direction == 0) ? servos[index].posDroit_pwm
                         : servos[index].posDevie_pwm;

    servos[index].softStartCounter = 1;
}

/* ============================================================
 * F1 — configure(index, posDroit_us, posDevie_us, speed_us)
 * ------------------------------------------------------------
 * Convertit les positions µs → PWM.
 * Convertit la vitesse µs → step PWM.
 * ============================================================ */
void EXSA_Servo::configure(uint8_t index,
                           uint16_t posDroit_us,
                           uint16_t posDevie_us,
                           uint16_t speed_us) noexcept
{
    if (!indexValide(index))
        return;

    servos[index].posDroit_pwm = usToPwm(posDroit_us);
    servos[index].posDevie_pwm = usToPwm(posDevie_us);

    // Conversion vitesse µs → PWM step
    servos[index].speed_pwm =
        (uint32_t)speed_us * 4095u / SERVO_US_RANGE / 50u;

    if (servos[index].speed_pwm < 1)
        servos[index].speed_pwm = 1;

    servos[index].current_pwm = servos[index].target_pwm;
    servos[index].softStartCounter = 1;
}

/* ============================================================
 * F2 — test(index)
 * ------------------------------------------------------------
 * Alterne entre droite et déviée autour du point médian.
 * ============================================================ */
void EXSA_Servo::test(uint8_t index) noexcept
{
    if (!indexValide(index))
        return;

    uint16_t mid = (servos[index].posDroit_pwm + servos[index].posDevie_pwm) / 2;

    servos[index].target_pwm =
        (servos[index].current_pwm < mid)
        ? servos[index].posDevie_pwm
        : servos[index].posDroit_pwm;

    servos[index].softStartCounter = 1;
}

/* ============================================================
 * Accesseurs (utilisés par EXSA_Switches)
 * ============================================================ */
bool EXSA_Servo::isMoving(uint8_t index) noexcept
{
    if (!indexValide(index))
        return false;

    return servos[index].current_pwm != servos[index].target_pwm;
}

uint16_t EXSA_Servo::getCurrentPwm(uint8_t index) noexcept
{
    if (!indexValide(index))
        return 0;

    return servos[index].current_pwm;
}

uint16_t EXSA_Servo::getTargetPwm(uint8_t index) noexcept
{
    if (!indexValide(index))
        return 0;

    return servos[index].target_pwm;
}

/* ============================================================
 * Setters des modes avancés
 * ============================================================ */
void EXSA_Servo::enableServoOff(bool enable) noexcept
{
    servoOffEnabled = enable;
}

void EXSA_Servo::enableSoftStart(bool enable) noexcept
{
    softStartEnabled = enable;
}

void EXSA_Servo::setDeadband(uint16_t value) noexcept
{
    deadband = value;
}
