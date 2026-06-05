#include "logic/Juego.h"

#include "logic/BarreraEstatica.h"
#include "logic/BarreraMovil.h"
#include "logic/BarreraTemporizada.h"
#include "logic/Nivel.h"
#include "logic/NivelDefensaNucleo.h"
#include "logic/NivelRutaTransmision.h"
#include "logic/SuperficieRebote.h"

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
    jugador_ = Jugador{Posicion{0.0F, 2.8F}};
    discoJugador_ = Disco{Posicion{0.0F, 2.8F}};

    auto nivel = std::make_unique<NivelRutaTransmision>(jugador_, discoJugador_);

    nivel->agregarTramo({"TRAMO A", 0.0F, 5.55F, 1.45F, 4.55F});
    nivel->agregarTramo({"TRAMO B", 5.9F, 11.45F, -3.75F, 3.15F});
    nivel->agregarTramo({"TRAMO C", 11.8F, 17.85F, -3.65F, 3.15F});
    nivel->agregarTramo({"TRAMO D", 18.15F, 22.4F, -3.65F, 3.1F});
    nivel->agregarTramo({"TRAMO E", 19.55F, 32.25F, -3.95F, -0.65F});
    nivel->agregarTramo({"TRAMO F", 32.6F, 39.45F, -3.85F, -0.75F});

    // Fases 2, 3, 4, 5 y 6: cinco nodos obligatorios antes del nodo central.
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-1",
        Posicion{5.2F, 2.8F},
        5400ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-2",
        Posicion{11.0F, -3.2F},
        6200ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-3",
        Posicion{17.5F, 2.8F},
        6500ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-4",
        Posicion{20.0F, -4.25F},
        7200ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-5",
        Posicion{30.4F, -2.25F},
        7000ms));

    for (const auto& checkpoint : checkpoints_) {
        nivel->agregarCheckpoint(*checkpoint);
    }

    metaFinal_ = std::make_unique<NodoCentralEnergia>(
        "nodo-central",
        Posicion{39.0F, -2.8F},
        6500ms);
    nivel->establecerMetaFinal(*metaFinal_);

    // Fase 1: compuerta de entrada con dos puertas verticales.
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{2.55F, 4.5F},
        Posicion{2.55F, 3.25F},
        1.35F,
        0.42F));
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{2.55F, 1.1F},
        Posicion{2.55F, 2.35F},
        1.35F,
        0.42F));

    // Fase 3: compuerta lateral en la subida hacia el checkpoint 3.
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{15.1F, 0.45F},
        Posicion{13.1F, 0.45F},
        1.1F,
        0.38F));
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{16.1F, 1.25F},
        Posicion{18.1F, 1.25F},
        1.1F,
        0.38F));

    // Fase 4: bloqueos que cierran el tiro directo desde checkpoint 3 hacia checkpoint 4.
    obstaculos_.push_back(std::make_unique<BarreraEstatica>(
        Posicion{18.5F, 0.25F},
        0.16F));
    // Fase 4: pared vertical rosada para rebotar desde checkpoint 3 hacia checkpoint 4.
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{23.0F, -1.31F},
        Posicion{-1.0F, 0.0F},
        0.66F));

    // Fase 5: rebote consecutivo para salir de checkpoint 4 hacia checkpoint 5.
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{25.15F, -0.95F},
        Posicion{0.223F, 0.975F},
        0.85F));
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{26.75F, -3.55F},
        Posicion{-0.332F, -0.943F},
        0.85F));

    for (const auto& obstaculo : obstaculos_) {
        nivel->agregarObstaculo(*obstaculo);
    }

    // Fase 2: dos drones verticales despues de checkpoint 1 hacia checkpoint 2.
    drones_.push_back(std::make_unique<Dron>(Posicion{6.8F, 2.0F}, 1.3F));
    drones_.back()->definirRutaVigilancia({
        Posicion{6.8F, 2.0F},
        Posicion{6.8F, -2.55F}
    });
    drones_.push_back(std::make_unique<Dron>(Posicion{8.7F, -2.55F}, 1.35F));
    drones_.back()->definirRutaVigilancia({
        Posicion{8.7F, -2.55F},
        Posicion{8.7F, 1.8F}
    });

    // Fase 3: dron horizontal en la subida desde checkpoint 2.
    drones_.push_back(std::make_unique<Dron>(Posicion{13.0F, -1.05F}, 1.35F));
    drones_.back()->definirRutaVigilancia({
        Posicion{12.0F, -1.05F},
        Posicion{14.0F, -1.05F}
    });

    // Fase 6: defensor rojo final, restringido al acceso del nodo central.
    drones_.push_back(std::make_unique<Dron>(Posicion{35.2F, -2.8F}, 2.05F));
    drones_.back()->definirRutaVigilancia({
        Posicion{34.35F, -1.55F},
        Posicion{36.05F, -3.35F},
        Posicion{34.35F, -3.35F},
        Posicion{36.05F, -1.55F}
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
