/*
 * Module : EXSA_Canton
 * Rôle   : Gestion des LEDs d’état du canton via PCA9685.
 *
 * Fonctionnement général :
 *   - Affiche l’état du canton :
 *       • OCCUPÉ      (rouge)
 *       • LIBRE       (vert)
 *       • MOUVEMENT   (orange, pulse court)
 *       • ERREUR      (rouge fixe ou clignotant)
 *
 *   - Gère deux mécanismes internes :
 *       • Animation de démarrage (séquence 4 LEDs)
 *       • Pulse mouvement (durée EXSA_CANTON_MOUVEMENT_MS)
 *
 *   - Le PCA9685 est partagé avec :
 *       • EXSA_Servo
 *       • EXSA_LedDirection
 *
 * Contraintes :
 *   - Aucune allocation dynamique
 *   - Fonctions très courtes (appelées dans la boucle 10–20 ms)
 *   - Animation non bloquante
 *
 * Notes :
 *   - L’animation masque temporairement l’état réel du canton.
 *   - Dès qu’elle est terminée, l’état LIBRE est affiché par défaut.
 */

#include "EXSA_Canton.h"
#include "EXSA_Config.h"

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Pointeur PCA9685 partagé (défini dans EXSA_Main)
// -----------------------------------------------------------------------------
Adafruit_PWMServoDriver *EXSA_Canton::pca = nullptr;

// -----------------------------------------------------------------------------
// Variables internes (animation + mouvement)
// -----------------------------------------------------------------------------
static uint8_t animStep = 0;
static unsigned long animTimer = 0;
static bool animEnCours = true;

static bool mouvementActif = false;
static unsigned long mouvementTimer = 0;

// -----------------------------------------------------------------------------
// Helpers PCA (inline = plus rapide)
// -----------------------------------------------------------------------------
static inline void ledOff(uint8_t ch) noexcept
{
    if (EXSA_Canton::pca)
        EXSA_Canton::pca->setPWM(ch, 0, 0);
}

static inline void ledOn(uint8_t ch, uint16_t pwm = 4095) noexcept
{
    if (EXSA_Canton::pca)
        EXSA_Canton::pca->setPWM(ch, 0, pwm);
}

// -----------------------------------------------------------------------------
// begin() — appelé par EXSA_Main
// ------------------------------------------------------------
// Initialise le module et lance l’animation de démarrage.
// -----------------------------------------------------------------------------
void EXSA_Canton::begin(Adafruit_PWMServoDriver *driver) noexcept
{
    initialiser(driver);
}

// -----------------------------------------------------------------------------
// setVoisins() — Compatibilité EA (voisins)
// ------------------------------------------------------------
// Réservé pour Discovery 2027 : EXSA_Canton n’utilise pas encore
// l’occupation des cantons adjacents.
// -----------------------------------------------------------------------------
void EXSA_Canton::setVoisins(uint8_t v) noexcept
{
    (void)v;
}

// -----------------------------------------------------------------------------
// initialiser() — Reset + animation de démarrage
// -----------------------------------------------------------------------------
void EXSA_Canton::initialiser(Adafruit_PWMServoDriver *driver) noexcept
{
    pca = driver;

    if (!pca)
        return;

    // Éteindre toutes les LEDs
    ledOff(PCA_CANTON_OCCUPE);
    ledOff(PCA_CANTON_LIBRE);
    ledOff(PCA_CANTON_MOUVEMENT);
    ledOff(PCA_CANTON_ERREUR);

    // Animation de démarrage
    animStep = 0;
    animTimer = millis();
    animEnCours = true;
}

// -----------------------------------------------------------------------------
// update() — animation + pulse mouvement
// ------------------------------------------------------------
// Appelé toutes les 10–20 ms par EXSA_Main.
// Gère :
//   - la séquence d’animation (4 étapes)
//   - l’extinction automatique du pulse mouvement
// -----------------------------------------------------------------------------
void EXSA_Canton::update() noexcept
{
    const unsigned long now = millis();

    // ---------------------------------------------------------
    // Animation de démarrage (séquence OCCUPÉ → LIBRE → MVT → ERR)
    // ---------------------------------------------------------
    if (animEnCours)
    {
        if (now - animTimer > EXSA_CANTON_ANIM_STEP_MS)
        {
            animTimer = now;

            // Tout OFF avant l’étape suivante
            ledOff(PCA_CANTON_OCCUPE);
            ledOff(PCA_CANTON_LIBRE);
            ledOff(PCA_CANTON_MOUVEMENT);
            ledOff(PCA_CANTON_ERREUR);

            switch (animStep)
            {
            case 0: ledOn(PCA_CANTON_OCCUPE); break;
            case 1: ledOn(PCA_CANTON_LIBRE); break;
            case 2: ledOn(PCA_CANTON_MOUVEMENT); break;
            case 3: ledOn(PCA_CANTON_ERREUR); break;

            case 4:
                // Fin animation → état initial = LIBRE
                ledOn(PCA_CANTON_LIBRE);
                animEnCours = false;
                break;
            }

            if (animStep < 4)
                animStep++;
        }
    }

    // ---------------------------------------------------------
    // Pulse mouvement (LED orange ON pendant X ms)
    // ---------------------------------------------------------
    if (mouvementActif && now - mouvementTimer > EXSA_CANTON_MOUVEMENT_MS)
    {
        mouvementActif = false;
        ledOff(PCA_CANTON_MOUVEMENT);
    }
}

// -----------------------------------------------------------------------------
// setOccupation() — OCCUPÉ / LIBRE
// ------------------------------------------------------------
// OCCUPÉ = rouge ON, vert OFF
// LIBRE  = vert ON, rouge OFF
//
// L’état n’est appliqué qu’après la fin de l’animation.
// -----------------------------------------------------------------------------
void EXSA_Canton::setOccupation(bool occupe) noexcept
{
    if (!animEnCours)
    {
        ledOn(PCA_CANTON_OCCUPE, occupe ? 4095 : 0);
        ledOn(PCA_CANTON_LIBRE,  occupe ? 0    : 4095);
    }
}

// -----------------------------------------------------------------------------
// pulseMouvement() — Flash orange court
// ------------------------------------------------------------
// Utilisé par EXSA_Essieux lors du passage d’un essieu.
// -----------------------------------------------------------------------------
void EXSA_Canton::pulseMouvement() noexcept
{
    mouvementActif = true;
    mouvementTimer = millis();
    ledOn(PCA_CANTON_MOUVEMENT);
}

// -----------------------------------------------------------------------------
// setErreur() — LED rouge erreur
// ------------------------------------------------------------
// Allume ou éteint la LED d’erreur (hors animation).
// -----------------------------------------------------------------------------
void EXSA_Canton::setErreur(bool erreur) noexcept
{
    if (!animEnCours)
    {
        ledOn(PCA_CANTON_ERREUR, erreur ? 4095 : 0);
    }
}

// -----------------------------------------------------------------------------
// debugCapteurs() — Mode debug (EXSA_DEBUG)
// ------------------------------------------------------------
// Permet d’allumer directement les 4 LEDs pour tests.
// -----------------------------------------------------------------------------
#if EXSA_DEBUG
void EXSA_Canton::debugCapteurs(bool occupe, bool libre, bool mouv, bool err) noexcept
{
    ledOn(PCA_CANTON_OCCUPE,    occupe ? 4095 : 0);
    ledOn(PCA_CANTON_LIBRE,     libre  ? 4095 : 0);
    ledOn(PCA_CANTON_MOUVEMENT, mouv   ? 4095 : 0);
    ledOn(PCA_CANTON_ERREUR,    err    ? 4095 : 0);
}
#endif

/* -------------------------------------------------------------
   Fin de EXSA_Canton.cpp
   -------------------------------------------------------------*/
