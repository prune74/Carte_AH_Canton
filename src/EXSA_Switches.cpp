/*
 * Module : EXSA_Switches
 * Rôle   : Lecture des micro‑switchs d’aiguilles + sécurité anti‑blocage.
 *
 * Fonctionnement général :
 *   - Chaque aiguille possède 2 micro‑switchs :
 *        • DROIT
 *        • DEVIÉ
 *
 *   - Le module déduit :
 *        • la position réelle : DROIT / DEVIÉ / INDET / INCOHÉRENT
 *        • l’état sécurité    : OK / BLOQUÉ / ERREUR
 *
 *   - Le module envoie au SA :
 *        • trame 0x06 (position + état)
 *          → sur changement immédiat
 *          → périodiquement (200 ms)
 *
 *   - Le module surveille les mouvements :
 *        • notifierMouvementDemarre() appelé par EXSA_Servo
 *        • calcul dynamique du temps max autorisé
 *        • détection blocage si dépassement
 *
 * Contraintes :
 *   - Aucun malloc
 *   - Lecture MCP23017 non bloquante
 *   - update() doit être très court (appelé ~1 kHz)
 *
 * Notes :
 *   - La sécurité est entièrement locale : même si le SA plante,
 *     EXSA_Switches continue de surveiller les aiguilles.
 */

#include "EXSA_Switches.h"
#include "EXSA_Pins.h"
#include "EXSA_Config.h"
#include "EXSA_UartTx.h"
#include "EXSA_Servo.h"
#include "Discovery_Protocol.h"

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

/*
 * ============================================================
 *  Variables statiques
 * ------------------------------------------------------------
 *  Ces tableaux mémorisent :
 *    - les derniers états envoyés au SA
 *    - les timers d’envoi périodique
 *    - les timers de détection de blocage
 *
 *  Ils sont définis ici (et non dans le header) car ils ne
 *  doivent exister qu’une seule fois dans tout le programme.
 * ============================================================
 */

Adafruit_MCP23X17 EXSA_Switches::mcp;

/*
 * Définition des tableaux constexpr déclarés dans le .h
 * ------------------------------------------------------------
 * ⚠️ Obligatoire sur ESP32 / GCC 8.4.0 :
 *    Un tableau static constexpr DOIT avoir une définition
 *    dans un .cpp, sinon → undefined reference.
 */
constexpr uint8_t EXSA_Switches::swDroit[AIG_COUNT] = {
    MCP_SW0_DROIT,
    MCP_SW1_DROIT,
    MCP_SW2_DROIT};

constexpr uint8_t EXSA_Switches::swDevie[AIG_COUNT] = {
    MCP_SW0_DEVIE,
    MCP_SW1_DEVIE,
    MCP_SW2_DEVIE};

// Derniers états envoyés au SA (position + état sécurité)
uint8_t EXSA_Switches::lastPos[AIG_COUNT] = {255, 255, 255};
uint8_t EXSA_Switches::lastEtat[AIG_COUNT] = {255, 255, 255};
uint32_t EXSA_Switches::lastSendMs[AIG_COUNT] = {0, 0, 0};

// Suivi du mouvement pour détection blocage
uint32_t EXSA_Switches::moveStartMs[AIG_COUNT] = {0, 0, 0};
uint32_t EXSA_Switches::moveMaxMs[AIG_COUNT] = {0, 0, 0};
bool EXSA_Switches::movementActive[AIG_COUNT] = {false, false, false};

/*
 * ============================================================
 *  Helper : calcul du temps max de mouvement
 * ------------------------------------------------------------
 *  Le temps max dépend :
 *    - de l’amplitude réelle du mouvement (PWM)
 *    - de la vitesse configurée
 *
 *  On ajoute une marge de sécurité.
 * ============================================================
 */
static uint32_t calculerTempsMaxMouvement(uint8_t idx)
{
    uint16_t cur = EXSA_Servo::getCurrentPwm(idx);
    uint16_t tgt = EXSA_Servo::getTargetPwm(idx);

    if (cur == 0 || tgt == 0)
        return 800;

    uint16_t amplitude = (cur > tgt) ? (cur - tgt) : (tgt - cur);

    uint32_t temps_theorique = amplitude / 2;

    if (temps_theorique < 200)
        temps_theorique = 200;
    if (temps_theorique > 1500)
        temps_theorique = 1500;

    return temps_theorique + 200;
}

/*
 * ============================================================
 *  begin()
 * ------------------------------------------------------------
 *  Configure les entrées MCP23017 pour les micro‑switchs.
 * ============================================================
 */
void EXSA_Switches::begin()
{
    mcp.begin_I2C(MCP23017_ADDR);

    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        mcp.pinMode(swDroit[i], INPUT_PULLUP);
        mcp.pinMode(swDevie[i], INPUT_PULLUP);
    }
}

/*
 * ============================================================
 *  notifierMouvementDemarre()
 * ------------------------------------------------------------
 *  Appelé par EXSA_Servo lorsqu’un mouvement F0 commence.
 *  Initialise le timer de surveillance anti‑blocage.
 * ============================================================
 */
void EXSA_Switches::notifierMouvementDemarre(uint8_t idx)
{
    if (idx >= AIG_COUNT)
        return;

    movementActive[idx] = true;
    moveStartMs[idx] = millis();
    moveMaxMs[idx] = calculerTempsMaxMouvement(idx);
}

/*
 * ============================================================
 *  lirePosition()
 * ------------------------------------------------------------
 *  Déduit la position réelle à partir des micro‑switchs.
 *
 *  Cas possibles :
 *    - DROIT
 *    - DEVIÉ
 *    - INDET (aucun switch actif)
 *    - INCOHÉRENT (les deux actifs)
 * ============================================================
 */
uint8_t EXSA_Switches::lirePosition(uint8_t idx)
{
    bool droit = !mcp.digitalRead(swDroit[idx]);
    bool devie = !mcp.digitalRead(swDevie[idx]);

    if (droit && !devie)
        return PROTO_POS_DROIT;
    if (!droit && devie)
        return PROTO_POS_DEVIE;
    if (!droit && !devie)
        return PROTO_POS_INDET;
    return PROTO_POS_INCOHERENT;
}

/*
 * ============================================================
 *  lireEtat()
 * ------------------------------------------------------------
 *  Déduit l’état sécurité :
 *    - OK
 *    - BLOQUÉ (mouvement trop long)
 *    - ERREUR (INDET ou INCOHÉRENT)
 * ============================================================
 */
uint8_t EXSA_Switches::lireEtat(uint8_t idx, uint8_t pos)
{
    if (pos == PROTO_POS_INCOHERENT)
        return PROTO_ETAT_ERREUR;

    if (pos == PROTO_POS_INDET)
        return PROTO_ETAT_ERREUR;

    if (movementActive[idx])
    {
        uint32_t now = millis();
        if (now - moveStartMs[idx] > moveMaxMs[idx])
            return PROTO_ETAT_BLOQUE;
    }

    return PROTO_ETAT_OK;
}

/*
 * ============================================================
 *  envoyerTrame()
 * ------------------------------------------------------------
 *  Envoie la trame 0x06 au SA.
 * ============================================================
 */
void EXSA_Switches::envoyerTrame(uint8_t idx, uint8_t pos, uint8_t etat)
{
    EXSA_UartTx::envoyerTramePositionAiguille(idx, pos, etat);
    lastSendMs[idx] = millis();
}

/*
 * ============================================================
 *  update()
 * ------------------------------------------------------------
 *  Boucle principale :
 *    - lit position + état
 *    - détecte fin de mouvement
 *    - envoie trame si changement ou périodiquement (200 ms)
 * ============================================================
 */
void EXSA_Switches::update()
{
    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        uint8_t pos = lirePosition(i);
        uint8_t etat = lireEtat(i, pos);

        // Si position stable et OK → fin de mouvement
        if ((pos == PROTO_POS_DROIT || pos == PROTO_POS_DEVIE) &&
            etat == PROTO_ETAT_OK)
        {
            movementActive[i] = false;
        }

        bool changed = (pos != lastPos[i]) || (etat != lastEtat[i]);
        bool periodic = (millis() - lastSendMs[i] >= 200);

        if (changed || periodic)
        {
            envoyerTrame(i, pos, etat);
            lastPos[i] = pos;
            lastEtat[i] = etat;
        }
    }
}
