/*
 * Module : EXSA_Main
 * Rôle   : Point d’entrée du firmware EXSA (Discovery 2026).
 *
 * Fonctionnement général :
 *   - Déclare toutes les instances globales EXSA :
 *       • PCA9685 (g_pca9685)
 *       • Multiplexeur (mux)
 *       • Signaux SNCF (signaux)
 *       • LED directionnelles (direction)
 *
 *   - Déclare les variables d’adressage EXSA :
 *       • exsaAdresse   (0 = H, 1 = AH)
 *       • exsaIsHoraire (sens horaire / antihoraire)
 *       • exsaHasBooster (présence booster)
 *
 *   - Ordonne le cycle de vie EXSA :
 *       • begin() → initialisation système (EXSA_System)
 *       • loop()  → exécution continue (EXSA_Runtime)
 *
 * Architecture :
 *   - EXSA_System::init() configure :
 *       • GPIO
 *       • MCP23017
 *       • PCA9685
 *       • Servos
 *       • Signaux
 *       • Canton
 *       • Booster (si présent)
 *
 *   - EXSA_Runtime::update() exécute :
 *       • lecture entrées
 *       • gestion servos
 *       • gestion signaux
 *       • gestion canton
 *       • gestion essieux
 *       • communication SA (UART)
 *       • tâches périodiques
 *
 * Notes :
 *   - EXSA_Main ne contient aucune logique métier.
 *   - Il sert uniquement de glue entre le système Arduino et l’architecture EXSA.
 */

#include "EXSA_Main.h"
#include "EXSA_System.h"
#include "EXSA_Runtime.h"
#include "EXSA_Pins.h"
#include "EXSA_Config.h"
#include "EXSA_Multiplexeur.h"

#include <Arduino.h>

// ============================================================
// Définition des variables globales (déclarées en extern)
// ------------------------------------------------------------
// Ces objets sont accessibles dans tout le firmware EXSA.
// ============================================================

// PCA9685 global (servos + LEDs)
Adafruit_PWMServoDriver g_pca9685 = Adafruit_PWMServoDriver();

// Adressage / mode EXSA
uint8_t exsaAdresse   = 0;     // 0 = H, 1 = AH
bool    exsaIsHoraire = true;  // sens horaire
bool    exsaHasBooster = false; // booster présent ?

// Multiplexeur global (sélection signaux / servos)
EXSA_Multiplexeur mux(PIN_MUX_P1, PIN_MUX_P2, PIN_MUX_P3, PIN_MUX_P4);

// Instances globales EXSA
EXSA_Signaux signaux(&mux, EXSA_SIGNAL_EST_MANOEUVRE);
EXSA_LedDirection direction(&g_pca9685, PCA_DIR_1, PCA_DIR_2, PCA_DIR_3, PCA_DIR_4);

// ============================================================
// Cycle de vie EXSA
// ============================================================

/*
 * begin()
 * ------------------------------------------------------------
 * Appelé une seule fois au démarrage.
 * Initialise tout le système via EXSA_System::init().
 */
void EXSA_Main::begin() noexcept
{
    EXSA_System::init();
}

/*
 * loop()
 * ------------------------------------------------------------
 * Boucle principale Arduino.
 * Déléguée entièrement à EXSA_Runtime::update().
 */
void EXSA_Main::loop() noexcept
{
    EXSA_Runtime::update();
}
