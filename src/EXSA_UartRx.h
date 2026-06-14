#pragma once
#include <stdint.h>
#include <HardwareSerial.h>

/*
 *  EXSA_UartRx.h — Réception UART SA → EXSA
 */

namespace EXSA_UartRx
{
    void begin(HardwareSerial& serial, uint32_t baudrate) noexcept;
    void process() noexcept;
}
