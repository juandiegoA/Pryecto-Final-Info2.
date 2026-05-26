#pragma once

#include "logic/Nivel.h"

class NivelDefensaNucleo final : public Nivel {
public:
    std::string nombre() const override;
    void actualizar() override;
};

