#pragma once

#include "logic/Obstaculo.h"
#include "logic/Posicion.h"

class BarreraMovil final : public Obstaculo {
public:
    BarreraMovil() = default;
    BarreraMovil(Posicion inicio, Posicion fin, float velocidad, float tolerancia);

    bool estaActivo() const noexcept override;
    void actualizar(float segundos) noexcept override;
    bool bloqueaAl(const Disco& disco) const noexcept override;

    const Posicion& posicion() const noexcept;

private:
    Posicion inicio_;
    Posicion fin_;
    Posicion posicion_;
    float velocidad_{1.0F};
    float tolerancia_{0.5F};
    bool haciaFin_{true};
};
