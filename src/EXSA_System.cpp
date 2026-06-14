/*
 * Module : EXSA_System
 * Rôle   : Initialisation complète du système EXSA (Discovery 2026).
 *
 * Fonctionnement général :
 *   - Lit les DIP (H/AH, Booster)
 *   - Configure l’UART RS485 (réception SA → EXSA)
 *   - Initialise tous les modules EXSA :
 *        • Servos
 *        • Canton (LEDs occupation + mouvement)
 *        • Micro‑switchs d’aiguilles
 *        • Quadrature essieux (ISR + queue)
 *        • Signaux SNCF
 *        • LED directionnelles
 *
 *   - Démarre la tâche BoosterCore si le booster est présent
 *     → Le booster gère :
 *         • DCC
 *         • Cutout RailCom
 *         • Mesure courant / tension
 *         • Détection OCCUPATION (courant)
 *
 * Notes Discovery 2026 :
 *   - Le capteur de présence GPIO n’existe plus.
 *   - L’occupation du canton est désormais détectée par le booster.
 *   - EXSA_Essieux ne gère plus l’occupation, uniquement :
 *        • DELTA AXE
 *        • PONCTUEL
 */

#include "EXSA_System.h"

#include "EXSA_Inputs.h"
#include "EXSA_Servo.h"
#include "EXSA_Switches.h"
#include "EXSA_Quadrature.h"
#include "EXSA_Canton.h"
#include "EXSA_Signaux.h"
#include "EXSA_LedDirection.h"
#include "EXSA_UartRx.h"
#include "EXSA_BoosterCore.h"
#include "EXSA_Calibration.h"
#include "EXSA_Pins.h"
#include "EXSA_Main.h"
#include "EXSA_Config.h"
#include "Discovery_Protocol.h"

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

// Instances globales déclarées dans EXSA_Main.cpp
extern Adafruit_PWMServoDriver g_pca9685;
extern bool exsaIsHoraire;
extern uint8_t exsaAdresse;
extern bool exsaHasBooster;

extern EXSA_Multiplexeur mux;
extern EXSA_Signaux signaux;
extern EXSA_LedDirection direction;

/* ============================================================
 * init()
 * ------------------------------------------------------------
 * Point d’entrée de l’initialisation EXSA.
 * Appelé une seule fois au démarrage.
 * ============================================================ */
void EXSA_System::init()
{
    // --------------------------------------------------------
    // 1) Lecture DIP H/AH (sens horaire / antihoraire)
    // --------------------------------------------------------
    exsaIsHoraire = EXSA_Inputs::readDipHoraire();
    exsaAdresse = exsaIsHoraire ? 0 : 1;

    // --------------------------------------------------------
    // 2) Lecture DIP Booster (présence booster)
    // --------------------------------------------------------
    exsaHasBooster = EXSA_Inputs::readDipBooster();

    // --------------------------------------------------------
    // 3) UART SA → EXSA (réception RS485)
    // --------------------------------------------------------
    EXSA_UartRx::begin(Serial1, EXSA_UART_BAUDRATE);

    // --------------------------------------------------------
    // 4) PCA9685 (servos + LEDs canton + LEDs directionnelles)
    // --------------------------------------------------------
    EXSA_Servo::begin();
    EXSA_Canton::begin(&g_pca9685);

    // --------------------------------------------------------
    // 5) MCP23017 (micro‑switchs d’aiguilles)
    // --------------------------------------------------------
    EXSA_Switches::begin();

    // --------------------------------------------------------
    // 6) Quadrature essieux (queue + ISR)
    // --------------------------------------------------------
    EXSA_Quadrature::initQueue();
    EXSA_Quadrature::installerInterruptions();

    // --------------------------------------------------------
    // 7) Signaux SNCF : aspect initial = MASQUÉ
    // --------------------------------------------------------
    (void)signaux.setAspect(ASPECT_MASQUE);

    // --------------------------------------------------------
    // 8) Calibration Booster — initialisation des seuils
    // --------------------------------------------------------
    EXSA_Calibration::init();

    // --------------------------------------------------------
    // 9) Booster (tâche FreeRTOS)
    // --------------------------------------------------------
    if (exsaHasBooster)
    {
        EXSA_BoosterCore::startTask();
    }
    else
    {
        // Pas de booster → LED d’erreur ON
        g_pca9685.setPWM(PCA_BOOSTER_ERROR, 0, 4095);
    }
}
