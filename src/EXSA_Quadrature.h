#pragma once
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/*
   EXSA_Quadrature.h — Version EXSA universel (architecture FreeRTOS)

   Gestion d’une seule paire de capteurs quadrature (A/B).
*/

class EXSA_Quadrature {
public:
    // Initialisation de la queue FreeRTOS
    static void initQueue() noexcept;

    // Installation des interruptions sur les pins A/B
    static void installerInterruptions() noexcept;

    // Lecture non bloquante d’un événement A/B dans la queue
    static bool lireEvenement(uint8_t &etat) noexcept;
};

// ISR ultra-courte appelée sur changement d’état des signaux A/B
void IRAM_ATTR isrQuadrature();
