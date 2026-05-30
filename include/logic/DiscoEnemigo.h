#pragma once

#include "logic/Disco.h"

class Jugador;

class DiscoEnemigo final : public Disco {
public:
    DiscoEnemigo() = default;
    explicit DiscoEnemigo(Posicion posicion);

    void avanzarHacia(const Jugador& jugador, float segundos, float velocidad) noexcept;
    void destruir() noexcept;
    bool estaDestruido() const noexcept;
    bool impactaAl(const Jugador& jugador, float tolerancia) const noexcept;

private:
    bool destruido_{false};
};
