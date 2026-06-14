#pragma once
#include <stdint.h>
#include "Discovery_Protocol.h"   // pour ExsaAspect

class EXSA_Multiplexeur;

/*
 * EXSA_Signaux.h — Version ENUM (Option A)
 */

class EXSA_Signaux
{
public:
    explicit EXSA_Signaux(EXSA_Multiplexeur *mux, bool estManoeuvre);

    [[nodiscard]] bool setAspect(ExsaAspect aspect);
    void update();

private:
    EXSA_Multiplexeur *mux;
    bool estManoeuvre;
    ExsaAspect aspectActuel;
};
