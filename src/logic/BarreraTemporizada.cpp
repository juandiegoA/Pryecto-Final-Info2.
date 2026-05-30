#include "logic/BarreraTemporizada.h"

#include "logic/Disco.h"

#include <algorithm>
#include <chrono>

BarreraTemporizada::BarreraTemporizada() = default;

BarreraTemporizada::BarreraTemporizada(Temporizador temporizador)
    : tiempoCerrada_(std::max(temporizador.duracion(), std::chrono::milliseconds{1})),
      tiempoAbierta_(tiempoCerrada_) {}

BarreraTemporizada::BarreraTemporizada(
    Posicion posicion,
    float tolerancia,
    std::chrono::milliseconds tiempoCerrada,
    std::chrono::milliseconds tiempoAbierta)
    : posicion_(posicion),
      tolerancia_(tolerancia >= 0.0F ? tolerancia : 0.0F),
      tiempoCerrada_(std::max(tiempoCerrada, std::chrono::milliseconds{1})),
      tiempoAbierta_(std::max(tiempoAbierta, std::chrono::milliseconds{1})) {}

bool BarreraTemporizada::estaActivo() const noexcept {
    return cerrada_;
}

void BarreraTemporizada::actualizar(float segundos) noexcept {
    if (segundos <= 0.0F) {
        return;
    }

    const auto intervalo = std::chrono::milliseconds{
        static_cast<std::chrono::milliseconds::rep>(segundos * 1000.0F)};
    avanzarFase(intervalo);
}

bool BarreraTemporizada::bloqueaAl(const Disco& disco) const noexcept {
    return estaActivo() && disco.cruzaPor(posicion_, tolerancia_);
}

const Posicion& BarreraTemporizada::posicion() const noexcept {
    return posicion_;
}

void BarreraTemporizada::avanzarFase(std::chrono::milliseconds intervalo) noexcept {
    while (intervalo.count() > 0) {
        const auto duracionActual = cerrada_ ? tiempoCerrada_ : tiempoAbierta_;
        const auto restante = duracionActual - transcurridoFase_;
        const auto avance = std::min(intervalo, restante);

        transcurridoFase_ += avance;
        intervalo -= avance;

        if (transcurridoFase_ >= duracionActual) {
            cerrada_ = !cerrada_;
            transcurridoFase_ = std::chrono::milliseconds{0};
        }
    }
}
