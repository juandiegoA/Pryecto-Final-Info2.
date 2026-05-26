#pragma once

#include "logic/Nivel.h"

class NivelRutaTransmision final : public Nivel {
public:
    std::string nombre() const override;
    void actualizar() override;
};

