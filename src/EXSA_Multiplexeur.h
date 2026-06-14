#pragma once
#include <stdint.h>

/*
 * EXSA_Multiplexeur.h
 * Pilote Charlieplexing multi‑instance pour signaux ferroviaires
 */

/*-------------------------------------------------------------
   IDs des LED du mât de signalisation (Charlieplexing)
--------------------------------------------------------------*/
enum ExsaLedId {
    LED_RALENTISSEMENT = 0,
    LED_RAPPEL,
    LED_SEMAPHORE,
    LED_LIBRE,
    LED_AVERTISSEMENT,
    LED_OEILLETON,
    LED_MANOEUVRE,
    LED_CARRE,
    LED_CARRE_VIOLET,
    LED_MAX
};

/*-------------------------------------------------------------
   Structure interne : état d'une LED
--------------------------------------------------------------*/
struct ExsaLedState {
    bool    allumer;
    uint8_t intensite;
    bool    clignote;
    bool    etatClignote;
};

/*-------------------------------------------------------------
   Classe EXSA_Multiplexeur
--------------------------------------------------------------*/
class EXSA_Multiplexeur {
public:
    EXSA_Multiplexeur(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);

    void reglerLed(ExsaLedId id, bool allumer,
                   uint8_t intensite = 255,
                   bool clignote = false);

    void reglerIntensite(ExsaLedId id, uint8_t intensite);

    void mettreAJour();

private:
    uint8_t P[4];
    ExsaLedState etats[LED_MAX];

    uint32_t dernierCligno;
    uint32_t dernierScan;
    uint8_t  indexScan;

    void configLigne(uint8_t pin, int mode, int valeur = 0);
    void appliquerLed(uint8_t index);
};
