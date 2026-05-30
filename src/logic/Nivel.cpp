#include "logic/Nivel.h"

Nivel::~Nivel() = default;

void Nivel::actualizar(std::chrono::milliseconds) {
    actualizar();
}

bool Nivel::victoria() const {
    return false;
}

bool Nivel::derrota() const {
    return false;
}
