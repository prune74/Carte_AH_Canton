#pragma once
#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>

/*
 * EXSA_LedDirection.h
 * ------------------------------------------------------------
 * Gestion des feux directionnels (barrettes cumulatives) via PCA9685.
 */

class EXSA_LedDirection
{
public:
    explicit EXSA_LedDirection(Adafruit_PWMServoDriver *driver,
                               uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4) noexcept;

    [[nodiscard]] bool setDirection(uint8_t direction) noexcept;

    void setIntensity(uint8_t index, uint8_t intensite) noexcept;

private:
    Adafruit_PWMServoDriver *pca;

    uint8_t C[4];
    uint8_t intensites[4];
    bool    etats[4];
};
