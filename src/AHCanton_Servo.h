#pragma once
#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>

class AHCanton_Servo
{
public:
    static void begin(Adafruit_PWMServoDriver *driver) noexcept;
    static void update() noexcept;

    // Commandes
    static void move(uint8_t index, bool voieA) noexcept;
    static void setTargetUs(uint8_t index, uint16_t us) noexcept;

    // Configuration
    static void configure(uint8_t index,
                          uint16_t posA_us,
                          uint16_t posB_us,
                          uint16_t speed_us) noexcept;

    // Tests
    static void test(uint8_t index) noexcept;

    // Accesseurs
    static bool     isMoving(uint8_t index) noexcept;
    static uint16_t getCurrentPwm(uint8_t index) noexcept;
    static uint16_t getTargetPwm(uint8_t index) noexcept;

    // Modes avancés
    static void enableServoOff(bool enable) noexcept;
    static void enableSoftStart(bool enable) noexcept;
    static void setDeadband(uint16_t value) noexcept;

    static constexpr uint8_t SERVO_COUNT = 6;
};
