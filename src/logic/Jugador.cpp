#include "logic/Jugador.h"

#include "logic/Disco.h"

Jugador::Jugador(Posicion posicion) : posicion_(posicion), posicionInicial_(posicion) {}

const Posicion& Jugador::posicion() const noexcept {
    return posicion_;
}

void Jugador::moverA(const Posicion& posicion) noexcept {
    teletransportarA(posicion);
}

void Jugador::teletransportarA(const Posicion& posicion) noexcept {
    posicion_ = posicion;
}

void Jugador::reiniciar() noexcept {
    posicion_ = posicionInicial_;
}

bool Jugador::puedeInteractuarCon(const Disco& disco, float tolerancia) const noexcept {
    return disco.colisionaCon(posicion_, tolerancia);
}
