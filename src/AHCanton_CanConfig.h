#pragma once
#include "CanConfig.h"
#include "AHCanton_Pins.h"

struct AHCanton_CanConfig : public CanConfigProvider {

    uint8_t busCount() const override {
        return 1; // Carte_AH_Canton = 1 bus CAN CANTON
    }

    const CanBusConfig& bus(uint8_t index) const override {
        static CanBusConfig cfg;

        cfg.enabled   = true;
        cfg.speed     = 500000; // 500 kbps pour CANTON

        // CAN intégré ESP32 (TWAI)
        cfg.tx_pin    = PIN_CAN_TX;
        cfg.rx_pin    = PIN_CAN_RX;

        // Pas de MCP2515
        cfg.cs_pin    = GPIO_NUM_NC;
        cfg.int_pin   = GPIO_NUM_NC;
        cfg.sck_pin   = GPIO_NUM_NC;
        cfg.mosi_pin  = GPIO_NUM_NC;
        cfg.miso_pin  = GPIO_NUM_NC;

        cfg.quartz    = 0;
        cfg.tolerance = 0;
        cfg.loopback  = false;

        return cfg;
    }
};
