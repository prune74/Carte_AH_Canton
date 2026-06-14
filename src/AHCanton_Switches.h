#pragma once
#include <stdint.h>
#include <Adafruit_MCP23X17.h>

class AHCanton_Switches
{
public:
    enum class Position : uint8_t
    {
        DROIT,
        DEVIE,
        INDET,
        INCOHERENT
    };

    enum class Etat : uint8_t
    {
        OK,
        BLOQUE,
        ERREUR
    };

    static constexpr uint8_t SWITCH_COUNT = 6; // 6 aiguilles → 12 fins de course

    static void begin(Adafruit_MCP23X17 *driver) noexcept;
    static void update() noexcept;

    // Lecture directe
    static Position lirePosition(uint8_t idx) noexcept;
    static Etat lireEtat(uint8_t idx, Position pos) noexcept;

    // Mouvement (appelé par AHCanton_Servo)
    static void notifierMouvementDemarre(uint8_t idx) noexcept;

    // Changement d’état (pour CAN)
    static bool hasChanged(uint8_t idx) noexcept;
    static Position getLastPosition(uint8_t idx) noexcept;
    static Etat getLastEtat(uint8_t idx) noexcept;

private:
    static Adafruit_MCP23X17 *mcp;

    static const uint8_t swDroit[SWITCH_COUNT];
    static const uint8_t swDevie[SWITCH_COUNT];

    static Position lastPos[SWITCH_COUNT];
    static Etat lastEtat[SWITCH_COUNT];
    static uint32_t lastSendMs[SWITCH_COUNT];

    static bool movementActive[SWITCH_COUNT];
    static uint32_t moveStartMs[SWITCH_COUNT];
    static uint32_t moveMaxMs[SWITCH_COUNT];

    static uint32_t calculerTempsMaxMouvement(uint8_t idx) noexcept;
};
