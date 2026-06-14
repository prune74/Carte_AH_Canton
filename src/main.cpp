/*
 * Module : main.cpp (EXSA)
 * Rôle   : Point d’entrée matériel du firmware EXSA (ESP32).
 *
 * Fonctionnement général :
 *   - Configure le port série debug (optionnel)
 *   - Initialise EXSA via EXSA_Main::begin()
 *   - Configure le watchdog matériel (WDT)
 *   - Crée la tâche FreeRTOS dédiée à EXSA
 *
 * Architecture :
 *   - Une seule tâche EXSA (exsaTask)
 *   - EXSA_Main::loop() est exécuté ~1000 fois par seconde
 *   - Le Booster, s’il est activé, tourne dans sa propre tâche
 *
 * Contraintes temps réel :
 *   - La tâche EXSA doit impérativement reset le watchdog
 *   - Aucun blocage dans EXSA_Main::loop()
 *   - La tâche tourne sur un cœur dédié (APP_CPU_NUM)
 *
 * Notes :
 *   - La loop() Arduino est volontairement inutilisée.
 *   - Toute la logique EXSA tourne dans FreeRTOS.
 */

#include <Arduino.h>
#include "EXSA_Main.h"
#include "EXSA_Config.h"
#include "esp_task_wdt.h"

static TaskHandle_t exsaTaskHandle = nullptr;

// Timeout watchdog (en secondes)
static constexpr int EXSA_WDT_TIMEOUT_SEC = 2;

/* ============================================================
 * exsaTask()
 * ------------------------------------------------------------
 * Tâche FreeRTOS principale EXSA.
 *
 * Étapes :
 *   1) Enregistre la tâche auprès du watchdog
 *   2) Boucle infinie :
 *        - reset watchdog
 *        - exécute EXSA_Main::loop()
 *        - délai 1 ms (cadence ~1 kHz)
 *
 * Cette tâche est le cœur du runtime EXSA.
 * ============================================================ */
void exsaTask(void* parameter)
{
#if EXSA_DEBUG
    Serial.println("[RTOS] Tâche EXSA démarrée");
#endif

    // Enregistrement auprès du watchdog
    esp_task_wdt_add(nullptr);

    for (;;)
    {
        esp_task_wdt_reset();      // sécurité temps réel
        EXSA_Main::loop();         // logique EXSA
        vTaskDelay(pdMS_TO_TICKS(1)); // cadence 1 kHz
    }
}

/* ============================================================
 * setup()
 * ------------------------------------------------------------
 * Point d’entrée Arduino.
 *
 * Étapes :
 *   1) Port série debug (optionnel)
 *   2) Initialisation EXSA (EXSA_Main::begin)
 *   3) Initialisation watchdog
 *   4) Création de la tâche EXSA
 *
 * Le Booster sera lancé automatiquement par EXSA_Main::begin()
 * si le DIP Booster est activé.
 * ============================================================ */
void setup()
{
#if EXSA_DEBUG
    Serial.begin(EXSA_UART_BAUDRATE);
    delay(50);
    Serial.println("\n[BOOT] EXSA démarrage (RTOS + WDT)...");
#endif

    // Initialisation complète EXSA (système + modules)
    EXSA_Main::begin();

    // Watchdog matériel
    esp_task_wdt_init(EXSA_WDT_TIMEOUT_SEC, true);

    // Création de la tâche EXSA
    xTaskCreatePinnedToCore(
        exsaTask,           // fonction
        "EXSA_Task",        // nom
        4096,               // stack
        nullptr,            // paramètre
        1,                  // priorité
        &exsaTaskHandle,    // handle
        APP_CPU_NUM         // cœur CPU
    );
}

/* ============================================================
 * loop()
 * ------------------------------------------------------------
 * Boucle Arduino standard : inutilisée.
 *
 * EXSA tourne exclusivement dans FreeRTOS.
 * ============================================================ */
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
