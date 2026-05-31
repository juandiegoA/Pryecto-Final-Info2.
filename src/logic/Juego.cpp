#include "logic/Juego.h"

#include "logic/BarreraEstatica.h"
#include "logic/BarreraTemporizada.h"
#include "logic/Nivel.h"
#include "logic/NivelDefensaNucleo.h"
#include "logic/NivelRutaTransmision.h"

#include <chrono>
#include <stdexcept>
#include <utility>

using namespace std::chrono_literals;

Juego::Juego()
    : jugador_(Posicion{0.0F, 0.0F}),
      discoJugador_(Posicion{0.0F, 0.0F}) {}

Juego::~Juego() = default;

Jugador& Juego::jugador() noexcept {
    return jugador_;
}

const Jugador& Juego::jugador() const noexcept {
    return jugador_;
}

Disco& Juego::discoJugador() noexcept {
    return discoJugador_;
}

const Disco& Juego::discoJugador() const noexcept {
    return discoJugador_;
}

void Juego::establecerNivel(std::unique_ptr<Nivel> nivel) {
    if (nivel == nullptr) {
        throw std::invalid_argument{"Juego no puede establecer un nivel activo nulo"};
    }

    nivelActual_ = std::move(nivel);
    tipoNivelActivo_ = TipoNivelActivo::Ninguno;
}

Nivel* Juego::nivelActual() noexcept {
    return nivelActual_.get();
}

const Nivel* Juego::nivelActual() const noexcept {
    return nivelActual_.get();
}

void Juego::crearNivelRutaTransmision() {
    limpiarEntidadesDeNivel();
    configurarNivelRutaTransmision();
}

void Juego::crearNivelDefensaNucleo() {
    limpiarEntidadesDeNivel();
    configurarNivelDefensaNucleo();
}

void Juego::establecerDificultadDefensa(DificultadDefensa dificultad) noexcept {
    dificultadDefensa_ = dificultad;
}

DificultadDefensa Juego::dificultadDefensa() const noexcept {
    return dificultadDefensa_;
}

void Juego::cambiarANivelRutaTransmision() {
    crearNivelRutaTransmision();
}

void Juego::cambiarANivelDefensaNucleo() {
    crearNivelDefensaNucleo();
}

void Juego::actualizar(std::chrono::milliseconds intervalo) {
    if (nivelActual_ != nullptr) {
        nivelActual_->actualizar(intervalo);
    }
}

bool Juego::nivelActivoFinalizado() const noexcept {
    return nivelActual_ != nullptr && nivelActual_->estaFinalizado();
}

bool Juego::nivelActivoVictoria() const noexcept {
    return nivelActual_ != nullptr && nivelActual_->victoria();
}

bool Juego::nivelActivoDerrota() const noexcept {
    return nivelActual_ != nullptr && nivelActual_->derrota();
}

void Juego::reiniciarNivelActivo() {
    switch (tipoNivelActivo_) {
    case TipoNivelActivo::RutaTransmision:
        crearNivelRutaTransmision();
        break;
    case TipoNivelActivo::DefensaNucleo:
        crearNivelDefensaNucleo();
        break;
    case TipoNivelActivo::Ninguno:
        break;
    }
}

void Juego::reiniciarPartida() {
    jugador_ = Jugador{Posicion{0.0F, 0.0F}};
    discoJugador_ = Disco{Posicion{0.0F, 0.0F}};
    crearNivelRutaTransmision();
}

void Juego::limpiarEntidadesDeNivel() {
    nivelActual_.reset();
    checkpoints_.clear();
    metaFinal_.reset();
    obstaculos_.clear();
    drones_.clear();
    discosEnemigos_.clear();
}

void Juego::configurarNivelRutaTransmision() {
    jugador_ = Jugador{Posicion{0.0F, 0.0F}};
    discoJugador_ = Disco{Posicion{0.0F, 0.0F}};

    auto nivel = std::make_unique<NivelRutaTransmision>(jugador_, discoJugador_);

    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-1",
        Posicion{4.0F, 0.0F},
        4200ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-2",
        Posicion{8.0F, 2.0F},
        5000ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-3",
        Posicion{11.0F, -1.0F},
        5200ms));

    for (const auto& checkpoint : checkpoints_) {
        nivel->agregarCheckpoint(*checkpoint);
    }

    metaFinal_ = std::make_unique<NodoCentralEnergia>(
        "nodo-central",
        Posicion{14.0F, 0.0F},
        5500ms);
    nivel->establecerMetaFinal(*metaFinal_);

    obstaculos_.push_back(std::make_unique<BarreraEstatica>(
        Posicion{6.0F, -1.5F},
        0.5F));
    obstaculos_.push_back(std::make_unique<BarreraTemporizada>(
        Posicion{6.5F, 1.0F},
        0.45F,
        1100ms,
        900ms));
    obstaculos_.push_back(std::make_unique<BarreraTemporizada>(
        Posicion{9.5F, 0.4F},
        0.45F,
        900ms,
        1200ms));

    for (const auto& obstaculo : obstaculos_) {
        nivel->agregarObstaculo(*obstaculo);
    }

    drones_.push_back(std::make_unique<Dron>(Posicion{5.0F, 2.8F}, 1.8F));
    drones_.back()->definirRutaVigilancia({
        Posicion{5.0F, 2.8F},
        Posicion{8.0F, 1.4F},
        Posicion{10.5F, -0.4F}
    });
    drones_.push_back(std::make_unique<Dron>(Posicion{12.0F, 1.4F}, 1.2F));
    drones_.back()->definirRutaVigilancia({
        Posicion{12.0F, 1.4F},
        Posicion{12.5F, -1.2F}
    });

    for (const auto& dron : drones_) {
        nivel->agregarDron(*dron);
    }

    nivelActual_ = std::move(nivel);
    tipoNivelActivo_ = TipoNivelActivo::RutaTransmision;
}

void Juego::configurarNivelDefensaNucleo() {
    jugador_ = Jugador{Posicion{0.0F, 0.0F}};
    discoJugador_ = Disco{Posicion{0.0F, 0.0F}};

    auto nivel = std::make_unique<NivelDefensaNucleo>(jugador_, 30000ms);
    nivel->configurarDificultad(dificultadDefensa_);

    switch (dificultadDefensa_) {
    case DificultadDefensa::Facil:
        discosEnemigos_.push_back(std::make_unique<DiscoEnemigo>(Posicion{0.0F, 8.8F}));
        discosEnemigos_.push_back(std::make_unique<DiscoEnemigo>(Posicion{2.6F, 9.5F}));
        break;
    case DificultadDefensa::Dificil:
        discosEnemigos_.push_back(std::make_unique<DiscoEnemigo>(Posicion{-3.2F, 9.8F}));
        discosEnemigos_.push_back(std::make_unique<DiscoEnemigo>(Posicion{0.0F, 10.7F}));
        discosEnemigos_.push_back(std::make_unique<DiscoEnemigo>(Posicion{3.2F, 9.8F}));
        break;
    case DificultadDefensa::Medio:
    default:
        discosEnemigos_.push_back(std::make_unique<DiscoEnemigo>(Posicion{-2.5F, 8.0F}));
        discosEnemigos_.push_back(std::make_unique<DiscoEnemigo>(Posicion{2.5F, 9.5F}));
        break;
    }

    for (const auto& discoEnemigo : discosEnemigos_) {
        nivel->agregarDiscoEnemigo(*discoEnemigo);
    }

    nivel->iniciar();
    nivelActual_ = std::move(nivel);
    tipoNivelActivo_ = TipoNivelActivo::DefensaNucleo;
}
