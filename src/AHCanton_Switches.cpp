#include "AHCanton_Switches.h"
#include "AHCanton_Servo.h"
#include "AHCanton_Pins.h"
#include <Arduino.h>

Adafruit_MCP23X17* AHCanton_Switches::mcp = nullptr;

const uint8_t AHCanton_Switches::swDroit[SWITCH_COUNT] = {
    MCP_SW0_DROIT,
    MCP_SW1_DROIT,
    MCP_SW2_DROIT,
    MCP_SW3_DROIT,
    MCP_SW4_DROIT,
    MCP_SW5_DROIT
};

const uint8_t AHCanton_Switches::swDevie[SWITCH_COUNT] = {
    MCP_SW0_DEVIE,
    MCP_SW1_DEVIE,
    MCP_SW2_DEVIE,
    MCP_SW3_DEVIE,
    MCP_SW4_DEVIE,
    MCP_SW5_DEVIE
};


AHCanton_Switches::Position AHCanton_Switches::lastPos[SWITCH_COUNT] = {
    Position::INDET, Position::INDET, Position::INDET,
    Position::INDET, Position::INDET, Position::INDET
};

AHCanton_Switches::Etat AHCanton_Switches::lastEtat[SWITCH_COUNT] = {
    Etat::ERREUR, Etat::ERREUR, Etat::ERREUR,
    Etat::ERREUR, Etat::ERREUR, Etat::ERREUR
};

uint32_t AHCanton_Switches::lastSendMs[SWITCH_COUNT] = {0,0,0,0,0,0};

bool     AHCanton_Switches::movementActive[SWITCH_COUNT] = {false,false,false,false,false,false};
uint32_t AHCanton_Switches::moveStartMs[SWITCH_COUNT]    = {0,0,0,0,0,0};
uint32_t AHCanton_Switches::moveMaxMs[SWITCH_COUNT]      = {0,0,0,0,0,0};

void AHCanton_Switches::begin(Adafruit_MCP23X17 *driver) noexcept
{
    mcp = driver;

    for (uint8_t i = 0; i < SWITCH_COUNT; i++)
    {
        mcp->pinMode(swDroit[i], INPUT_PULLUP);
        mcp->pinMode(swDevie[i], INPUT_PULLUP);
    }
}

void AHCanton_Switches::notifierMouvementDemarre(uint8_t idx) noexcept
{
    if (idx >= SWITCH_COUNT)
        return;

    movementActive[idx] = true;
    moveStartMs[idx] = millis();
    moveMaxMs[idx] = calculerTempsMaxMouvement(idx);
}

AHCanton_Switches::Position AHCanton_Switches::lirePosition(uint8_t idx) noexcept
{
    bool droit = !mcp->digitalRead(swDroit[idx]);
    bool devie = !mcp->digitalRead(swDevie[idx]);

    if (droit && !devie) return Position::DROIT;
    if (!droit && devie) return Position::DEVIE;
    if (!droit && !devie) return Position::INDET;
    return Position::INCOHERENT;
}

AHCanton_Switches::Etat AHCanton_Switches::lireEtat(uint8_t idx, Position pos) noexcept
{
    if (pos == Position::INCOHERENT || pos == Position::INDET)
        return Etat::ERREUR;

    if (movementActive[idx])
    {
        uint32_t now = millis();
        if (now - moveStartMs[idx] > moveMaxMs[idx])
            return Etat::BLOQUE;
    }

    return Etat::OK;
}

uint32_t AHCanton_Switches::calculerTempsMaxMouvement(uint8_t idx) noexcept
{
    uint16_t cur = AHCanton_Servo::getCurrentPwm(idx);
    uint16_t tgt = AHCanton_Servo::getTargetPwm(idx);

    if (cur == 0 || tgt == 0)
        return 800;

    uint16_t amplitude = (cur > tgt) ? (cur - tgt) : (tgt - cur);

    uint32_t theorique = amplitude / 2;

    if (theorique < 200) theorique = 200;
    if (theorique > 1500) theorique = 1500;

    return theorique + 200;
}

bool AHCanton_Switches::hasChanged(uint8_t idx) noexcept
{
    uint32_t now = millis();
    return (now - lastSendMs[idx] >= 200);
}

AHCanton_Switches::Position AHCanton_Switches::getLastPosition(uint8_t idx) noexcept
{
    return lastPos[idx];
}

AHCanton_Switches::Etat AHCanton_Switches::getLastEtat(uint8_t idx) noexcept
{
    return lastEtat[idx];
}

void AHCanton_Switches::update() noexcept
{
    for (uint8_t i = 0; i < SWITCH_COUNT; i++)
    {
        Position pos = lirePosition(i);
        Etat etat = lireEtat(i, pos);

        bool changed = (pos != lastPos[i]) || (etat != lastEtat[i]);
        bool periodic = (millis() - lastSendMs[i] >= 200);

        if (changed || periodic)
        {
            lastPos[i] = pos;
            lastEtat[i] = etat;
            lastSendMs[i] = millis();
        }

        if (etat == Etat::OK &&
            (pos == Position::DROIT || pos == Position::DEVIE))
        {
            movementActive[i] = false;
        }
    }
}
