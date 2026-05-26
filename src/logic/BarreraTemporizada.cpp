#include "logic/BarreraTemporizada.h"

BarreraTemporizada::BarreraTemporizada(Temporizador temporizador)
    : temporizador_(temporizador) {}

bool BarreraTemporizada::estaActivo() const noexcept {
    return temporizador_.estaActivo();
}

