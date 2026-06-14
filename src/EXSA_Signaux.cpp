/*
 * Module : EXSA_Signaux
 * Rôle   : Gestion des aspects SNCF du mât Discovery 2026.
 *
 * Fonctionnement général :
 *   - Reçoit un aspect (ENUM ExsaAspect)
 *   - Éteint toutes les LED du mât
 *   - Active uniquement les LED correspondant à l’aspect demandé
 *   - Gère automatiquement :
 *        • clignotements (60, rappel 60, défaut)
 *        • intensités (via EXSA_Multiplexeur)
 *        • mode manœuvre (carré violet)
 *
 * Flux entrants :
 *   - setAspect() appelé par EXSA_Callbacks (E6/E7)
 *
 * Flux sortants :
 *   - Appels à EXSA_Multiplexeur::reglerLed()
 *
 * Contraintes :
 *   - Aucune logique temporelle ici (gérée par le multiplexeur)
 *   - Aucune allocation dynamique
 *   - Fonction setAspect() doit être très rapide
 *
 * Notes :
 *   - Le multiplexeur gère le PWM, le clignotement et le scan.
 *   - Ce module ne fait que définir *quelles* LED doivent être allumées.
 */

#include "EXSA_Signaux.h"
#include "EXSA_Multiplexeur.h"
#include "EXSA_Config.h"
#include "Discovery_Protocol.h"

#include <Arduino.h>

/* ------------------------------------------------------------
 * Constructeur
 * ------------------------------------------------------------
 * m          : pointeur vers le multiplexeur Charlieplexing
 * manoeuvre  : indique si le mât est un signal de manœuvre
 *
 * aspectActuel = ASPECT_DEFAUT → œilleton clignotant
 * ------------------------------------------------------------ */
EXSA_Signaux::EXSA_Signaux(EXSA_Multiplexeur* m, bool manoeuvre)
    : mux(m),
      estManoeuvre(manoeuvre),
      aspectActuel(ASPECT_DEFAUT)
{
}

/* ------------------------------------------------------------
 * setAspect(aspect)
 * ------------------------------------------------------------
 * Reçoit un aspect SNCF (ENUM) et applique les LED correspondantes.
 *
 * Étapes :
 *   1) Si l’aspect est identique → rien à faire
 *   2) Éteindre toutes les LED
 *   3) Allumer uniquement celles correspondant à l’aspect
 *
 * Retour :
 *   - true  si l’aspect a changé
 *   - false si identique
 *
 * Le multiplexeur se charge ensuite :
 *   - du PWM
 *   - du clignotement
 *   - du scan Charlieplexing
 * ------------------------------------------------------------ */
bool EXSA_Signaux::setAspect(ExsaAspect aspect)
{
    if (aspect == aspectActuel)
        return false;

    aspectActuel = aspect;

    // 1) Tout éteindre
    for (uint8_t i = 0; i < LED_MAX; i++)
        mux->reglerLed(static_cast<ExsaLedId>(i), false);

    // 2) Sélection des LED selon l’aspect
    switch (aspect)
    {
        case ASPECT_CARRE:
            if (estManoeuvre) {
                // Carré violet (mât de manœuvre)
                mux->reglerLed(LED_CARRE_VIOLET, true);
            } else {
                // Carré rouge (2 rouges)
                mux->reglerLed(LED_SEMAPHORE, true);
                mux->reglerLed(LED_CARRE, true);
            }
            break;

        case ASPECT_SEMAPHORE:
            mux->reglerLed(LED_SEMAPHORE, true);
            mux->reglerLed(LED_OEILLETON, true);
            break;

        case ASPECT_AVERTISSEMENT:
            mux->reglerLed(LED_AVERTISSEMENT, true);
            break;

        case ASPECT_RALENTISSEMENT_30:
            mux->reglerLed(LED_RALENTISSEMENT, true);
            break;

        case ASPECT_RALENTISSEMENT_60:
            // Clignotement
            mux->reglerLed(LED_RALENTISSEMENT, true, 0, true);
            break;

        case ASPECT_RAPPEL_30:
            mux->reglerLed(LED_RAPPEL, true);
            break;

        case ASPECT_RAPPEL_60:
            // Clignotement
            mux->reglerLed(LED_RAPPEL, true, 0, true);
            break;

        case ASPECT_VOIE_LIBRE:
            mux->reglerLed(LED_LIBRE, true);
            break;

        case ASPECT_MANOEUVRE:
            mux->reglerLed(LED_MANOEUVRE, true);
            break;

        case ASPECT_MASQUE:
            // Rien : toutes LED OFF
            break;

        case ASPECT_DEFAUT:
        default:
            // Œilleton clignotant
            mux->reglerLed(LED_OEILLETON, true, 0, true);
            break;
    }

    return true;
}

/* ------------------------------------------------------------
 * update()
 * ------------------------------------------------------------
 * Délégué au multiplexeur :
 *   - clignotement
 *   - PWM
 *   - scan Charlieplexing
 * ------------------------------------------------------------ */
void EXSA_Signaux::update()
{
    mux->mettreAJour();
}
