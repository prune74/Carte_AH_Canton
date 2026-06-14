#pragma once
#include <stdint.h>

/*
 * Module : EXSA_Calibration
 * Rôle   : Auto‑calibration NON BLOQUANTE du courant à vide.
 *
 * Fonctionnement :
 *   - start()   : lance une calibration (voie supposée vide)
 *   - process() : appelé toutes les 1 ms (BoosterCore)
 *   - isDone()  : indique que les seuils sont prêts
 *
 * API :
 *   - getSeuilLibre()
 *   - getSeuilOccupe()
 *   - setSeuils() (ex : depuis SA)
 */

namespace EXSA_Calibration
{
    void init();

    void start();        // lance une calibration
    void process();      // à appeler toutes les 1 ms

    bool isRunning();
    bool isDone();

    uint16_t getSeuilLibre();
    uint16_t getSeuilOccupe();

    void setSeuils(uint16_t libre, uint16_t occupe);
}
