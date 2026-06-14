#include "AHCanton_Servo.h"
#include <Arduino.h>

namespace
{
    Adafruit_PWMServoDriver *pca = nullptr;

    constexpr uint8_t SERVO_CHANNEL[AHCanton_Servo::SERVO_COUNT] = {
        0, 1, 2, 3, 4, 5   // À adapter selon ton PCB
    };

    constexpr uint16_t SERVO_US_MIN   = 800;
    constexpr uint16_t SERVO_US_MAX   = 2400;
    constexpr uint16_t SERVO_US_RANGE = SERVO_US_MAX - SERVO_US_MIN;

    bool servoOffEnabled  = true;
    bool softStartEnabled = true;
    uint16_t deadband     = 3;

    inline uint16_t usToPwm(uint16_t us)
    {
        if (us < SERVO_US_MIN) us = SERVO_US_MIN;
        if (us > SERVO_US_MAX) us = SERVO_US_MAX;
        return (uint32_t)(us - SERVO_US_MIN) * 4095u / SERVO_US_RANGE;
    }

    struct ServoData {
        uint16_t posA_pwm;
        uint16_t posB_pwm;
        uint16_t speed_pwm;
        uint16_t current_pwm;
        uint16_t target_pwm;
        uint16_t softStartCounter = 1;
    };

    ServoData servos[AHCanton_Servo::SERVO_COUNT];

    inline bool indexValid(uint8_t i)
    {
        return i < AHCanton_Servo::SERVO_COUNT;
    }

    inline void applyPwm(uint8_t i)
    {
        pca->setPWM(SERVO_CHANNEL[i], 0, servos[i].current_pwm);
    }
}

void AHCanton_Servo::begin(Adafruit_PWMServoDriver *driver) noexcept
{
    pca = driver;

    for (uint8_t i = 0; i < SERVO_COUNT; ++i)
    {
        servos[i].posA_pwm = usToPwm(1500);
        servos[i].posB_pwm = usToPwm(1500);
        servos[i].speed_pwm = 10;

        servos[i].current_pwm = servos[i].posA_pwm;
        servos[i].target_pwm  = servos[i].posA_pwm;

        servos[i].softStartCounter = 1;

        applyPwm(i);
    }
}

void AHCanton_Servo::update() noexcept
{
    for (uint8_t i = 0; i < SERVO_COUNT; ++i)
    {
        uint16_t cur = servos[i].current_pwm;
        uint16_t tgt = servos[i].target_pwm;

        if (abs((int)cur - (int)tgt) <= deadband)
        {
            servos[i].current_pwm = tgt;

            if (servoOffEnabled)
                pca->setPWM(SERVO_CHANNEL[i], 0, 0);
            else
                applyPwm(i);

            continue;
        }

        uint16_t step = servos[i].speed_pwm;

        if (softStartEnabled)
        {
            if (servos[i].softStartCounter < step)
                servos[i].softStartCounter++;

            step = servos[i].softStartCounter;
        }

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

        applyPwm(i);
    }
}

void AHCanton_Servo::move(uint8_t index, bool voieA) noexcept
{
    if (!indexValid(index))
        return;

    servos[index].target_pwm = voieA ? servos[index].posA_pwm
                                     : servos[index].posB_pwm;

    servos[index].softStartCounter = 1;
}

void AHCanton_Servo::setTargetUs(uint8_t index, uint16_t us) noexcept
{
    if (!indexValid(index))
        return;

    servos[index].target_pwm = usToPwm(us);
    servos[index].softStartCounter = 1;
}

void AHCanton_Servo::configure(uint8_t index,
                               uint16_t posA_us,
                               uint16_t posB_us,
                               uint16_t speed_us) noexcept
{
    if (!indexValid(index))
        return;

    servos[index].posA_pwm = usToPwm(posA_us);
    servos[index].posB_pwm = usToPwm(posB_us);

    servos[index].speed_pwm =
        (uint32_t)speed_us * 4095u / SERVO_US_RANGE / 50u;

    if (servos[index].speed_pwm < 1)
        servos[index].speed_pwm = 1;

    servos[index].softStartCounter = 1;
}

void AHCanton_Servo::test(uint8_t index) noexcept
{
    if (!indexValid(index))
        return;

    uint16_t mid = (servos[index].posA_pwm + servos[index].posB_pwm) / 2;

    servos[index].target_pwm =
        (servos[index].current_pwm < mid)
        ? servos[index].posB_pwm
        : servos[index].posA_pwm;

    servos[index].softStartCounter = 1;
}

bool AHCanton_Servo::isMoving(uint8_t index) noexcept
{
    if (!indexValid(index))
        return false;

    return servos[index].current_pwm != servos[index].target_pwm;
}

uint16_t AHCanton_Servo::getCurrentPwm(uint8_t index) noexcept
{
    return indexValid(index) ? servos[index].current_pwm : 0;
}

uint16_t AHCanton_Servo::getTargetPwm(uint8_t index) noexcept
{
    return indexValid(index) ? servos[index].target_pwm : 0;
}

void AHCanton_Servo::enableServoOff(bool enable) noexcept
{
    servoOffEnabled = enable;
}

void AHCanton_Servo::enableSoftStart(bool enable) noexcept
{
    softStartEnabled = enable;
}

void AHCanton_Servo::setDeadband(uint16_t value) noexcept
{
    deadband = value;
}
