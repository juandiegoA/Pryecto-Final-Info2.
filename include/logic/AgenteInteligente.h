#pragma once

#include "logic/Posicion.h"

class AgenteInteligente {
public:
    AgenteInteligente() = default;
    explicit AgenteInteligente(Posicion posicion);

    const Posicion& posicion() const noexcept;

private:
    Posicion posicion_;
};

