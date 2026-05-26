#pragma once

#include "logic/Obstaculo.h"

class BarreraEstatica final : public Obstaculo {
public:
    bool estaActivo() const noexcept override;
};

