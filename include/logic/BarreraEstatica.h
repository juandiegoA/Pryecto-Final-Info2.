#pragma once

#include "logic/Obstaculo.h"
#include "logic/Posicion.h"

class BarreraEstatica final : public Obstaculo {
public:
    BarreraEstatica() = default;
    BarreraEstatica(Posicion posicion, float tolerancia);

    bool estaActivo() const noexcept override;
    bool bloqueaAl(const Disco& disco) const noexcept override;

    const Posicion& posicion() const noexcept;

private:
    Posicion posicion_;
    float tolerancia_{0.5F};
};
