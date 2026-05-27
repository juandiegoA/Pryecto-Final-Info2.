#pragma once

#include "logic/Obstaculo.h"
#include "logic/Posicion.h"
#include "logic/Temporizador.h"

#include <chrono>

class BarreraTemporizada final : public Obstaculo {
public:
    BarreraTemporizada();
    explicit BarreraTemporizada(Temporizador temporizador);
    BarreraTemporizada(
        Posicion posicion,
        float tolerancia,
        std::chrono::milliseconds tiempoCerrada,
        std::chrono::milliseconds tiempoAbierta);

    bool estaActivo() const noexcept override;
    void actualizar(float segundos) noexcept override;
    bool bloqueaAl(const Disco& disco) const noexcept override;

    const Posicion& posicion() const noexcept;

private:
    void avanzarFase(std::chrono::milliseconds intervalo) noexcept;

    Posicion posicion_;
    float tolerancia_{0.5F};
    std::chrono::milliseconds tiempoCerrada_{1000};
    std::chrono::milliseconds tiempoAbierta_{1000};
    std::chrono::milliseconds transcurridoFase_{0};
    bool cerrada_{true};
};
