/*
 * Module : EXSA_LedDirection
 * Rôle   : Gestion des feux directionnels (jusqu’à 4 LED) via PCA9685.
 *
 * Fonctionnement général :
 *   - Chaque LED correspond à un canal PCA9685
 *   - Intensité individuelle configurable (0–255 → 0–4095 PWM)
 *   - Affichage d’un “niveau directionnel” cumulatif :
 *        0 = aucune LED
 *        1 = LED0
 *        2 = LED0 + LED1
 *        3 = LED0 + LED1 + LED2
 *        4 = LED0 + LED1 + LED2 + LED3
 *
 * Flux entrants :
 *   - setDirection() appelé par EXSA_Main lors des trames E8/E9
 *   - setIntensity() pour ajuster la luminosité de chaque LED
 *
 * Flux sortants :
 *   - Commandes PWM vers le PCA9685 partagé
 *
 * Contraintes :
 *   - Aucune allocation dynamique
 *   - Fonctions très courtes (appelées en temps réel)
 *   - Le PCA9685 est partagé avec EXSA_Servo et EXSA_Canton
 *
 * Notes :
 *   - Le module ne gère aucune logique métier : il applique simplement
 *     un niveau directionnel demandé par le SA.
 */

#include "EXSA_LedDirection.h"
#include "EXSA_Config.h"

/* ------------------------------------------------------------
 * Constructeur
 * ------------------------------------------------------------
 * driver : pointeur vers le PCA9685 partagé
 * c1..c4 : numéros de canaux PCA pour les 4 LED directionnelles
 *
 * Initialise :
 *   - les canaux
 *   - les états ON/OFF
 *   - les intensités par défaut (définies dans EXSA_Config)
 *   - toutes les LED éteintes
 * ------------------------------------------------------------ */
EXSA_LedDirection::EXSA_LedDirection(Adafruit_PWMServoDriver *driver,
                                     uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) noexcept
    : pca(driver)
{
    C[0] = c1;
    C[1] = c2;
    C[2] = c3;
    C[3] = c4;

    // États initiaux
    etats[0] = etats[1] = etats[2] = etats[3] = false;

    // Intensités configurables (0–255)
    intensites[0] = EXSA_DIR_LED0_INTENSITE;
    intensites[1] = EXSA_DIR_LED1_INTENSITE;
    intensites[2] = EXSA_DIR_LED2_INTENSITE;
    intensites[3] = EXSA_DIR_LED3_INTENSITE;

    // LEDs OFF au démarrage
    for (uint8_t i = 0; i < 4; i++)
        pca->setPWM(C[i], 0, 0);
}

/* ------------------------------------------------------------
 * setDirection(direction)
 * ------------------------------------------------------------
 * Applique un niveau directionnel cumulatif :
 *   0 → toutes LED OFF
 *   1 → LED0
 *   2 → LED0 + LED1
 *   3 → LED0 + LED1 + LED2
 *   4 → LED0 + LED1 + LED2 + LED3
 *
 * Retourne :
 *   - true  si l’affichage a changé
 *   - false si identique (évite recalcul inutile)
 * ------------------------------------------------------------ */
bool EXSA_LedDirection::setDirection(uint8_t direction) noexcept
{
    if (direction > 4)
        direction = 4;

    bool changed = false;

    for (uint8_t i = 0; i < 4; i++)
    {
        const bool on = (i < direction);

        if (etats[i] != on)
            changed = true;

        etats[i] = on;

        // Conversion intensité 0–255 → PWM 0–4095
        const uint16_t pwm = on ? static_cast<uint16_t>(intensites[i]) * 16u : 0u;

        pca->setPWM(C[i], 0, pwm);
    }

    return changed;
}

/* ------------------------------------------------------------
 * setIntensity(index, intensite)
 * ------------------------------------------------------------
 * Change l’intensité d’une LED directionnelle (0–255).
 *
 * Si la LED est ON :
 *   → applique immédiatement la nouvelle intensité.
 *
 * Si la LED est OFF :
 *   → la nouvelle intensité sera utilisée lors du prochain ON.
 * ------------------------------------------------------------ */
void EXSA_LedDirection::setIntensity(uint8_t index, uint8_t intensite) noexcept
{
    if (index >= 4)
        return;

    intensites[index] = intensite;

    if (etats[index])
    {
        const uint16_t pwm = static_cast<uint16_t>(intensite) * 16u;
        pca->setPWM(C[index], 0, pwm);
    }
}

/* ============================================================
   Fin de EXSA_LedDirection.cpp
   ============================================================ */
