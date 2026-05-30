#include "logic/BarreraEstatica.h"

#include "logic/Disco.h"

BarreraEstatica::BarreraEstatica(Posicion posicion, float tolerancia)
    : posicion_(posicion), tolerancia_(tolerancia >= 0.0F ? tolerancia : 0.0F) {}

bool BarreraEstatica::estaActivo() const noexcept {
    return true;
}

bool BarreraEstatica::bloqueaAl(const Disco& disco) const noexcept {
    return disco.cruzaPor(posicion_, tolerancia_);
}

const Posicion& BarreraEstatica::posicion() const noexcept {
    return posicion_;
}
