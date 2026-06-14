#pragma once
#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>

/*
 * EXSA_Canton.h
 * ------------------------------------------------------------
 * Module responsable de l’affichage de l’état d’un canton.
 * Version optimisée (header minimal).
 */

class EXSA_Canton {
public:
    static void begin(Adafruit_PWMServoDriver* driver) noexcept;
    static void initialiser(Adafruit_PWMServoDriver* driver) noexcept;
    static void update() noexcept;

    static void setOccupation(bool occupe) noexcept;
    static void pulseMouvement() noexcept;
    static void setErreur(bool erreur) noexcept;
    static void setVoisins(uint8_t voisins) noexcept;

#if EXSA_DEBUG
    static void debugCapteurs(bool occupe, bool libre, bool mouv, bool err) noexcept;
#endif

    // Pointeur PCA9685 utilisé par les helpers internes
    static Adafruit_PWMServoDriver* pca;
};
