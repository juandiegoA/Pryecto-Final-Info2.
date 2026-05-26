#include "logic/Juego.h"

#include "logic/Nivel.h"

#include <utility>

Juego::Juego() = default;

Juego::~Juego() = default;

Jugador& Juego::jugador() noexcept {
    return jugador_;
}

const Jugador& Juego::jugador() const noexcept {
    return jugador_;
}

void Juego::establecerNivel(std::unique_ptr<Nivel> nivel) {
    nivelActual_ = std::move(nivel);
}

Nivel* Juego::nivelActual() noexcept {
    return nivelActual_.get();
}
