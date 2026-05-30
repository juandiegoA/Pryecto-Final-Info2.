#include "logic/Checkpoint.h"

#include "logic/Disco.h"

#include <algorithm>
#include <utility>

Checkpoint::Checkpoint(Posicion posicion) : posicion_(posicion) {}

Checkpoint::Checkpoint(
    std::string id,
    Posicion posicion,
    std::chrono::milliseconds tiempoMaximo)
    : id_(std::move(id)),
      posicion_(posicion),
      tiempoMaximo_(std::max(tiempoMaximo, std::chrono::milliseconds{0})) {}

const std::string& Checkpoint::id() const noexcept {
    return id_;
}

const Posicion& Checkpoint::posicion() const noexcept {
    return posicion_;
}

std::chrono::milliseconds Checkpoint::tiempoMaximo() const noexcept {
    return tiempoMaximo_;
}

bool Checkpoint::estaActivado() const noexcept {
    return activado_;
}

void Checkpoint::activar() noexcept {
    activado_ = true;
}

void Checkpoint::desactivar() noexcept {
    activado_ = false;
}

bool Checkpoint::verificarLlegada(const Disco& disco, float tolerancia) const noexcept {
    return disco.cruzaPor(posicion_, tolerancia);
}
