#pragma once

#include "logic/Posicion.h"

class Disco;

class Jugador {
public:
    Jugador() = default;
    explicit Jugador(Posicion posicion);

    const Posicion& posicion() const noexcept;
    void moverA(const Posicion& posicion) noexcept;
    void teletransportarA(const Posicion& posicion) noexcept;
    void reiniciar() noexcept;
    bool puedeInteractuarCon(const Disco& disco, float tolerancia) const noexcept;

private:
    Posicion posicion_;
    Posicion posicionInicial_;
};
