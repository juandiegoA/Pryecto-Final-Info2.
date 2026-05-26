#pragma once

#include "logic/Obstaculo.h"
#include "logic/Temporizador.h"

class BarreraTemporizada final : public Obstaculo {
public:
    BarreraTemporizada() = default;
    explicit BarreraTemporizada(Temporizador temporizador);

    bool estaActivo() const noexcept override;

private:
    Temporizador temporizador_;
};

