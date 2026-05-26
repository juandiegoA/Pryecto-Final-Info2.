#include "logic/Temporizador.h"

#include <algorithm>

Temporizador::Temporizador(std::chrono::milliseconds duracion)
    : duracion_(std::max(duracion, std::chrono::milliseconds{0})) {}

void Temporizador::iniciar() noexcept {
    transcurrido_ = std::chrono::milliseconds{0};
    activo_ = duracion_.count() > 0;
}

void Temporizador::actualizar(std::chrono::milliseconds intervalo) noexcept {
    if (!activo_ || intervalo.count() <= 0) {
        return;
    }

    transcurrido_ = std::min(transcurrido_ + intervalo, duracion_);
    if (estaAgotado()) {
        activo_ = false;
    }
}

void Temporizador::reiniciar() noexcept {
    iniciar();
}

void Temporizador::detener() noexcept {
    activo_ = false;
}

bool Temporizador::estaActivo() const noexcept {
    return activo_;
}

bool Temporizador::estaAgotado() const noexcept {
    return transcurrido_ >= duracion_;
}

std::chrono::milliseconds Temporizador::duracion() const noexcept {
    return duracion_;
}

std::chrono::milliseconds Temporizador::tiempoTranscurrido() const noexcept {
    return transcurrido_;
}

std::chrono::milliseconds Temporizador::tiempoRestante() const noexcept {
    return duracion_ - transcurrido_;
}
