/*
 * Module : EXSA_Inputs
 * Rôle   : Gestion centralisée des entrées EXSA via MCP23017.
 *
 * Fonctionnement général :
 *   - Configure le MCP23017 en mode INPUT_PULLUP
 *   - Fournit une API simple pour lire :
 *       • DIP H/AH        (sens horaire / antihoraire)
 *       • DIP Booster     (booster présent / absent)
 *       • Toute entrée brute (readRaw)
 *
 * Notes Discovery 2026 :
 *   - Le capteur de présence n’est plus utilisé.
 *   - L’occupation vient désormais du booster (mesure de courant).
 */

#include "EXSA_Inputs.h"
#include "EXSA_Pins.h"

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Instance MCP23017 (statique, partagée par tout EXSA)
// -----------------------------------------------------------------------------
Adafruit_MCP23X17 EXSA_Inputs::mcp;

/* ============================================================
 * begin() — Initialisation du MCP23017
 * ------------------------------------------------------------
 * Configure uniquement :
 *   - DIP H/AH
 *   - DIP Booster
 *
 * Le capteur de présence est supprimé en Discovery 2026.
 * ============================================================ */
void EXSA_Inputs::begin()
{
    mcp.begin_I2C(MCP23017_ADDR);

    mcp.pinMode(MCP_DIP_HAH,     INPUT_PULLUP);
    mcp.pinMode(MCP_DIP_BOOSTER, INPUT_PULLUP);

    // MCP_PRESENCE supprimé → plus configuré
}

/* ============================================================
 * readDipHoraire()
 * ------------------------------------------------------------
 * Retourne l’état du DIP H/AH.
 * true  = horaire
 * false = antihoraire
 * ============================================================ */
bool EXSA_Inputs::readDipHoraire()
{
    return mcp.digitalRead(MCP_DIP_HAH);
}

/* ============================================================
 * readDipBooster()
 * ------------------------------------------------------------
 * Retourne l’état du DIP Booster.
 * true  = booster présent
 * false = booster absent
 * ============================================================ */
bool EXSA_Inputs::readDipBooster()
{
    return mcp.digitalRead(MCP_DIP_BOOSTER);
}

/* ============================================================
 * readRaw(pin)
 * ------------------------------------------------------------
 * Lecture brute d’une entrée MCP23017.
 * Utile pour debug ou extensions futures.
 * ============================================================ */
bool EXSA_Inputs::readRaw(uint8_t pin)
{
    return mcp.digitalRead(pin);
}
