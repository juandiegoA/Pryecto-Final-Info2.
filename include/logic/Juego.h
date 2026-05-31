#pragma once

#include "logic/Checkpoint.h"
#include "logic/DificultadDefensa.h"
#include "logic/Disco.h"
#include "logic/DiscoEnemigo.h"
#include "logic/Dron.h"
#include "logic/Jugador.h"
#include "logic/NodoCentralEnergia.h"
#include "logic/Obstaculo.h"

#include <chrono>
#include <memory>
#include <vector>

class Nivel;

// Coordina el estado principal sin depender de la representacion grafica.
class Juego {
public:
    Juego();
    ~Juego();

    Jugador& jugador() noexcept;
    const Jugador& jugador() const noexcept;
    Disco& discoJugador() noexcept;
    const Disco& discoJugador() const noexcept;
    void establecerNivel(std::unique_ptr<Nivel> nivel);
    Nivel* nivelActual() noexcept;
    const Nivel* nivelActual() const noexcept;

    void crearNivelRutaTransmision();
    void crearNivelDefensaNucleo();
    void establecerDificultadDefensa(DificultadDefensa dificultad) noexcept;
    DificultadDefensa dificultadDefensa() const noexcept;
    void cambiarANivelRutaTransmision();
    void cambiarANivelDefensaNucleo();
    void actualizar(std::chrono::milliseconds intervalo);
    bool nivelActivoFinalizado() const noexcept;
    bool nivelActivoVictoria() const noexcept;
    bool nivelActivoDerrota() const noexcept;
    void reiniciarNivelActivo();
    void reiniciarPartida();

private:
    void limpiarEntidadesDeNivel();
    void configurarNivelRutaTransmision();
    void configurarNivelDefensaNucleo();

    Jugador jugador_;
    Disco discoJugador_;
    std::unique_ptr<Nivel> nivelActual_;
    DificultadDefensa dificultadDefensa_{DificultadDefensa::Medio};
    enum class TipoNivelActivo {
        Ninguno,
        RutaTransmision,
        DefensaNucleo
    };
    TipoNivelActivo tipoNivelActivo_{TipoNivelActivo::Ninguno};

    std::vector<std::unique_ptr<Checkpoint>> checkpoints_;
    std::unique_ptr<NodoCentralEnergia> metaFinal_;
    std::vector<std::unique_ptr<Obstaculo>> obstaculos_;
    std::vector<std::unique_ptr<Dron>> drones_;
    std::vector<std::unique_ptr<DiscoEnemigo>> discosEnemigos_;
};
