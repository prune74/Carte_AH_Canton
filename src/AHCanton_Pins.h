#pragma once
#include <Arduino.h>

/* ============================================================
   AHCanton_Pins.h — Mappage matériel Carte_AH_Canton
   ============================================================ */

/* -------------------------
   I2C (PCA9685 + MCP23017)
   ------------------------- */
static const gpio_num_t PIN_I2C_SDA = GPIO_NUM_21;
static const gpio_num_t PIN_I2C_SCL = GPIO_NUM_22;

/* -------------------------
   WS2815 (RMT)
   ------------------------- */
static const gpio_num_t PIN_WS2815_DATA = GPIO_NUM_18;

/* -------------------------
   CAN (ESP32 natif)
   ------------------------- */
static const gpio_num_t PIN_CAN_RX = GPIO_NUM_4;
static const gpio_num_t PIN_CAN_TX = GPIO_NUM_5;

/* -------------------------
   PCA9685 — 6 servos
   ------------------------- */
static const uint8_t PCA_SERVO_0 = 0;
static const uint8_t PCA_SERVO_1 = 1;
static const uint8_t PCA_SERVO_2 = 2;
static const uint8_t PCA_SERVO_3 = 3;
static const uint8_t PCA_SERVO_4 = 4;
static const uint8_t PCA_SERVO_5 = 5;

/* -------------------------
   MCP23017 — 12 fins de course
   ------------------------- */
static const uint8_t MCP23017_ADDR = 0x20;

static const uint8_t MCP_SW0_DROIT = 0;
static const uint8_t MCP_SW0_DEVIE = 1;
static const uint8_t MCP_SW1_DROIT = 2;
static const uint8_t MCP_SW1_DEVIE = 3;
static const uint8_t MCP_SW2_DROIT = 4;
static const uint8_t MCP_SW2_DEVIE = 5;
static const uint8_t MCP_SW3_DROIT = 6;
static const uint8_t MCP_SW3_DEVIE = 7;
static const uint8_t MCP_SW4_DROIT = 8;
static const uint8_t MCP_SW4_DEVIE = 9;
static const uint8_t MCP_SW5_DROIT = 10;
static const uint8_t MCP_SW5_DEVIE = 11;
