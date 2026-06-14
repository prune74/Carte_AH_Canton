/*
 * Module : EXSA_Quadrature
 * Rôle   : Capture temps réel des signaux quadrature A/B.
 *
 * Fonctionnement général :
 *   - Deux GPIO (A et B) génèrent des interruptions sur CHANGE.
 *   - L’ISR lit immédiatement l’état brut (A<<1 | B).
 *   - L’état est envoyé dans une queue FreeRTOS (non bloquante).
 *   - La tâche EXSA (EXSA_Main / EXSA_Essieux) lit ensuite les
 *     événements via lireEvenement() et calcule le delta essieu.
 *
 * Architecture :
 *   - ISR ultra courte (aucun calcul, aucune logique)
 *   - Queue FreeRTOS pour découpler ISR ↔ logique métier
 *   - installerInterruptions() configure les GPIO + attachInterrupt
 *   - initQueue() crée la queue (32 événements)
 *
 * Contraintes temps réel :
 *   - ISR doit être la plus courte possible
 *   - Aucun malloc, aucune opération lourde
 *   - Queue non bloquante (xQueueSendFromISR)
 *
 * Notes :
 *   - Le filtrage, la détection de sens et le comptage sont faits
 *     dans EXSA_Essieux, jamais dans l’ISR.
 *   - Le module est volontairement minimaliste pour garantir
 *     une stabilité parfaite à haute fréquence.
 */

#include "EXSA_Quadrature.h"
#include "EXSA_Pins.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// -----------------------------------------------------------------------------
// Queue FreeRTOS pour transmettre les événements A/B
// -----------------------------------------------------------------------------
// Chaque événement = 1 octet : (A<<1 | B)
// La queue permet de bufferiser les fronts rapides sans bloquer l’ISR.
// -----------------------------------------------------------------------------
static QueueHandle_t quadQueue = nullptr;

/* ============================================================
 * ISR : capture brute A/B
 * ------------------------------------------------------------
 * Appelée sur chaque front CHANGE des signaux A et B.
 *
 * Étapes :
 *   1) lecture immédiate des GPIO
 *   2) construction de l’état (2 bits)
 *   3) envoi dans la queue (non bloquant)
 *   4) demande éventuelle de rescheduling
 *
 * Aucun calcul, aucune logique métier → ISR ultra rapide.
 * ============================================================ */
void IRAM_ATTR isrQuadrature()
{
    uint8_t a = digitalRead(PIN_QUAD_A);
    uint8_t b = digitalRead(PIN_QUAD_B);
    uint8_t etat = (a << 1) | b;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(quadQueue, &etat, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
}

/* ============================================================
 * initQueue()
 * ------------------------------------------------------------
 * Crée la queue FreeRTOS utilisée pour stocker les événements.
 * Taille : 32 états A/B.
 * ============================================================ */
void EXSA_Quadrature::initQueue() noexcept
{
    quadQueue = xQueueCreate(32, sizeof(uint8_t));
}

/* ============================================================
 * lireEvenement()
 * ------------------------------------------------------------
 * Lecture non bloquante d’un événement A/B.
 * Retourne true si un état est disponible.
 * ============================================================ */
bool EXSA_Quadrature::lireEvenement(uint8_t &etat) noexcept
{
    return xQueueReceive(quadQueue, &etat, 0) == pdTRUE;
}

/* ============================================================
 * installerInterruptions()
 * ------------------------------------------------------------
 * Configure les GPIO A/B en entrée et attache les interruptions.
 *
 * Les interruptions sont déclenchées sur CHANGE :
 *   - front montant
 *   - front descendant
 *
 * L’état initial est lu mais non utilisé (réservé debug).
 * ============================================================ */
void EXSA_Quadrature::installerInterruptions() noexcept
{
    pinMode(PIN_QUAD_A, INPUT);
    pinMode(PIN_QUAD_B, INPUT);

    uint8_t etatInitial =
        (digitalRead(PIN_QUAD_A) << 1) |
        digitalRead(PIN_QUAD_B);

    (void)etatInitial; // réservé pour debug futur

    attachInterrupt(PIN_QUAD_A, isrQuadrature, CHANGE);
    attachInterrupt(PIN_QUAD_B, isrQuadrature, CHANGE);
}
