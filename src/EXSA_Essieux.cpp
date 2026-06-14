/*
 * Module : EXSA_Essieux
 * Rôle   : Gestion unifiée de la détection d’essieux pour Discovery 2026.
 *
 * Fonctionnement général :
 *   - Détecte les mouvements ponctuels via EXSA_Quadrature (delta +1 / -1)
 *   - Gère un état PONCTUEL (actif quelques centaines de ms)
 *   - Informe le SA via EXSA_UartTx :
 *        • Trame DELTA AXE (+1 / -1)
 *        • Trame PONCTUEL (début / fin)
 *
 * Notes Discovery 2026 :
 *   - L’OCCUPATION physique n’est plus gérée ici.
 *   - L’OCCUPATION vient désormais du booster (mesure de courant).
 *   - Ce module ne gère plus aucune entrée GPIO.
 */

#include "EXSA_Essieux.h"
#include "EXSA_UartTx.h"
#include "EXSA_Config.h"
#include "EXSA_Canton.h"

#include <Arduino.h>

/* ============================================================
   Variables internes
   ------------------------------------------------------------
   ponctuelActif       : état du mouvement ponctuel
   dernierMouvementMs  : timestamp du dernier delta essieu
   ============================================================ */

static bool ponctuelActif = false;
static unsigned long dernierMouvementMs = 0;

static constexpr unsigned long PONCTUEL_TIMEOUT_MS = 200;

/* ============================================================
   onDeltaAxe(delta)
   ------------------------------------------------------------
   Appelé par EXSA_Quadrature lorsqu’un essieu est détecté.
   delta = +1 ou -1.
   Rôle :
     - Envoi immédiat du delta au SA
     - Pulse LED mouvement
     - Activation du PONCTUEL si nécessaire
   ============================================================ */
void EXSA_Essieux::onDeltaAxe(int delta) noexcept
{
    // 1) Envoi immédiat au SA
    EXSA_UartTx::envoyerTrameDeltaAxe(delta);

    // 2) LED mouvement locale
    EXSA_Canton::pulseMouvement();

    // 3) Activation du PONCTUEL
    if (!ponctuelActif)
    {
        ponctuelActif = true;
        EXSA_UartTx::envoyerTramePonctuel(true);
    }

    // Mise à jour du timestamp
    dernierMouvementMs = millis();
}

/* ============================================================
   update()
   ------------------------------------------------------------
   Appelé régulièrement dans EXSA_Main::loop().
   Rôle :
     - Éteindre automatiquement le PONCTUEL après timeout
   ============================================================ */
void EXSA_Essieux::update() noexcept
{
    const unsigned long now = millis();

    if (ponctuelActif && (now - dernierMouvementMs > PONCTUEL_TIMEOUT_MS))
    {
        ponctuelActif = false;
        EXSA_UartTx::envoyerTramePonctuel(false);
    }
}

/* ============================================================
   GETTERS (debug)
   ------------------------------------------------------------
   Utilisés uniquement pour affichage ou tests.
   ============================================================ */
bool EXSA_Essieux::getPonctuelActif() noexcept { return ponctuelActif; }
