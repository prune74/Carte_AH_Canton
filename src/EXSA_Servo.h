#pragma once
#include <stdint.h>

/*
 *  EXSA_Servo.h 
 */

namespace EXSA_Servo
{
    /* Cycle de vie */
    void begin() noexcept;
    void update() noexcept;

    /* Commandes SA → EXSA */
    void move(uint8_t index, uint8_t direction) noexcept;

    void configure(uint8_t index,
                   uint16_t posDroit_us,
                   uint16_t posDevie_us,
                   uint16_t speed_us) noexcept;

    void test(uint8_t index) noexcept;

    /* Modes avancés */
    void enableServoOff(bool enable) noexcept;
    void enableSoftStart(bool enable) noexcept;
    void setDeadband(uint16_t value) noexcept;

    /* Accesseurs pour EXSA_Switches */
    bool     isMoving(uint8_t index) noexcept;
    uint16_t getCurrentPwm(uint8_t index) noexcept;
    uint16_t getTargetPwm(uint8_t index) noexcept;
}
