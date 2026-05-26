#pragma once

#include "logic/Jugador.h"

#include <memory>

class Nivel;

// Coordina el estado principal sin depender de la representacion grafica.
class Juego {
public:
    Juego();
    ~Juego();

    Jugador& jugador() noexcept;
    const Jugador& jugador() const noexcept;
    void establecerNivel(std::unique_ptr<Nivel> nivel);
    Nivel* nivelActual() noexcept;

private:
    Jugador jugador_;
    std::unique_ptr<Nivel> nivelActual_;
};

