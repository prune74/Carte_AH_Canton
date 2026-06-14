/*
 * Module : EXSA_Runtime
 * Rôle   : Boucle d’exécution principale EXSA (hors Booster).
 *
 * Fonctionnement général (Discovery 2026) :
 *   - Appelé en continu par EXSA_Main::loop()
 *   - Exécute toutes les tâches temps réel EXSA :
 *        1) Lecture quadrature (file FreeRTOS)
 *        2) Gestion essieux (PONCTUEL, delta)
 *        3) Mise à jour signaux SNCF
 *        4) Mise à jour canton (LEDs)
 *        5) Mise à jour aiguilles (anti‑blocage)
 *
 * Notes Discovery 2026 :
 *   - L’occupation physique n’est plus gérée ici.
 *   - L’occupation vient désormais du booster (mesure de courant).
 *   - EXSA_Runtime ne lit plus aucun capteur de présence.
 *   - Le Booster tourne dans sa propre tâche FreeRTOS.
 */

#include "EXSA_Runtime.h"

#include "EXSA_Inputs.h"
#include "EXSA_Essieux.h"
#include "EXSA_Quadrature.h"
#include "EXSA_Signaux.h"
#include "EXSA_Canton.h"
#include "EXSA_Switches.h"
#include "EXSA_Main.h"

#include <Arduino.h>

// Instances globales déclarées dans EXSA_Main.cpp
extern EXSA_Signaux signaux;
extern EXSA_LedDirection direction;
extern bool exsaHasBooster;

// Dernier état quadrature (pour calcul delta)
static uint8_t quadEtatPrecedent = 0;

/* ============================================================
 * update()
 * ------------------------------------------------------------
 * Boucle principale EXSA (hors booster).
 * Appelée en continu par EXSA_Main::loop().
 * ============================================================ */
void EXSA_Runtime::update()
{
    /* --------------------------------------------------------
       1) Quadrature (file d’événements)
       --------------------------------------------------------
       Lecture non bloquante de la queue FreeRTOS.
       Chaque événement = état brut A/B (2 bits).
       On compare avec l’état précédent pour déterminer :
         +1 = mouvement horaire
         -1 = mouvement antihoraire
    -------------------------------------------------------- */
    uint8_t nouvelEtat;
    while (EXSA_Quadrature::lireEvenement(nouvelEtat))
    {
        int8_t delta = 0;

        // Séquence quadrature horaire
        if ((quadEtatPrecedent == 0 && nouvelEtat == 1) ||
            (quadEtatPrecedent == 1 && nouvelEtat == 3) ||
            (quadEtatPrecedent == 3 && nouvelEtat == 2) ||
            (quadEtatPrecedent == 2 && nouvelEtat == 0))
        {
            delta = +1;
        }
        // Séquence quadrature antihoraire
        else if ((quadEtatPrecedent == 0 && nouvelEtat == 2) ||
                 (quadEtatPrecedent == 2 && nouvelEtat == 3) ||
                 (quadEtatPrecedent == 3 && nouvelEtat == 1) ||
                 (quadEtatPrecedent == 1 && nouvelEtat == 0))
        {
            delta = -1;
        }

        quadEtatPrecedent = nouvelEtat;

        if (delta != 0)
            EXSA_Essieux::onDeltaAxe(delta);
    }

    /* --------------------------------------------------------
       2) Mise à jour des modules EXSA
       --------------------------------------------------------
       - Essieux : extinction PONCTUEL
       - Signaux : clignotements, transitions
       - Canton  : animation + pulse mouvement
       - Switches : anti‑blocage servos
    -------------------------------------------------------- */
    EXSA_Essieux::update();
    signaux.update();
    EXSA_Canton::update();
    EXSA_Switches::update();

    /* --------------------------------------------------------
       3) Mode passif : EXSA sans booster
       --------------------------------------------------------
       Si le DIP indique “pas de booster”, EXSA ne gère que :
         - signaux
         - canton
         - essieux
         - aiguilles
    -------------------------------------------------------- */
    if (!exsaHasBooster)
        return;

    /* --------------------------------------------------------
       4) Mode actif : Booster géré dans EXSA_BoosterCore
       --------------------------------------------------------
       Rien à faire ici : la tâche BoosterCore tourne en parallèle.
    -------------------------------------------------------- */
}
