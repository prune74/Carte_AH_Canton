#pragma once
#include <stdint.h>

/*
   EXSA_Essieux.h — Version Discovery 2026
   Module responsable de :
     - détection des deltas d’essieux (+1 / -1)
     - gestion du mouvement ponctuel (PONCTUEL)
     - envoi des trames UART vers le SA :
         • PROTO_05_DELTA_AXE
         • PROTO_03_PONCTUEL
     - gestion des LEDs de mouvement du canton

   Notes :
     - L’occupation physique n’est plus gérée ici.
     - L’occupation vient désormais du booster (mesure de courant).
*/

class EXSA_Essieux
{
public:
    // Appelé par EXSA_Quadrature lorsqu’un essieu est détecté
    static void onDeltaAxe(int delta) noexcept;

    // Appelé régulièrement dans EXSA_Main::loop()
    static void update() noexcept;

    // Debug uniquement
    [[nodiscard]] static bool getPonctuelActif() noexcept;
};
