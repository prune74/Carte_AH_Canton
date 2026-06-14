#pragma once
#include <stdint.h>
#include <HardwareSerial.h>
#include <driver/gpio.h>

/*
 * EXSA_RS485.h — Gestion RS485 (DE/RE + UART)
 */

namespace EXSA_RS485
{
    void begin(HardwareSerial &serial,
               uint32_t baudrate,
               gpio_num_t pinDE_RE) noexcept;

    void send(const uint8_t *data, uint8_t len) noexcept;

    HardwareSerial &uart() noexcept;
}
