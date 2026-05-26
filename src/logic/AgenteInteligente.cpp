#include "logic/AgenteInteligente.h"

AgenteInteligente::AgenteInteligente(Posicion posicion) : posicion_(posicion) {}

const Posicion& AgenteInteligente::posicion() const noexcept {
    return posicion_;
}

