#include "logic/Dron.h"

Dron::Dron(Posicion posicion) : posicion_(posicion) {}

const Posicion& Dron::posicion() const noexcept {
    return posicion_;
}

