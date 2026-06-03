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
    jugador_ = Jugador{Posicion{0.0F, 2.0F}};
    discoJugador_ = Disco{Posicion{0.0F, 2.0F}};

    auto nivel = std::make_unique<NivelRutaTransmision>(jugador_, discoJugador_);

    nivel->agregarTramo({"INICIO", 0.0F, 3.35F, 0.95F, 3.25F});
    nivel->agregarTramo({"DESCENSO", 3.75F, 9.9F, -3.35F, 2.65F});
    nivel->agregarTramo({"SUBIDA", 10.2F, 15.95F, -3.25F, 2.55F});
    nivel->agregarTramo({"REBOTE A CP4", 16.0F, 19.3F, -3.15F, 2.45F});
    nivel->agregarTramo({"DOBLE REBOTE", 16.0F, 24.7F, -3.45F, -0.75F});
    nivel->agregarTramo({"FINAL", 25.2F, 30.75F, -3.35F, -0.4F});

    // Fases 2, 3, 4, 5 y 6: cinco nodos obligatorios antes del nodo central.
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-1",
        Posicion{4.75F, 2.0F},
        5400ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-2",
        Posicion{9.35F, -2.85F},
        6200ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-3",
        Posicion{15.8F, 1.8F},
        6500ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-4",
        Posicion{16.2F, -2.4F},
        7200ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-5",
        Posicion{24.2F, -2.65F},
        7000ms));

    for (const auto& checkpoint : checkpoints_) {
        nivel->agregarCheckpoint(*checkpoint);
    }

    metaFinal_ = std::make_unique<NodoCentralEnergia>(
        "nodo-central",
        Posicion{30.1F, -2.55F},
        6500ms);
    nivel->establecerMetaFinal(*metaFinal_);

    // Fase 1: compuerta de entrada con dos puertas verticales.
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{2.45F, 3.2F},
        Posicion{2.45F, 2.35F},
        1.15F,
        0.5F));
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{2.45F, 0.8F},
        Posicion{2.45F, 1.65F},
        1.15F,
        0.5F));

    // Fase 3: compuerta lateral en la subida hacia el checkpoint 3.
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{13.25F, 0.7F},
        Posicion{12.25F, 0.7F},
        0.75F,
        0.46F));
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{14.15F, 1.2F},
        Posicion{15.15F, 1.2F},
        0.75F,
        0.46F));

    // Fase 4: bloqueos que cierran el tiro directo desde checkpoint 3 hacia checkpoint 4.
    obstaculos_.push_back(std::make_unique<BarreraEstatica>(
        Posicion{15.95F, 0.05F},
        0.42F));
    obstaculos_.push_back(std::make_unique<BarreraEstatica>(
        Posicion{16.25F, -1.05F},
        0.42F));

    // Fase 5: bloqueos que hacen necesaria la ruta de doble rebote hacia checkpoint 5.
    obstaculos_.push_back(std::make_unique<BarreraEstatica>(
        Posicion{19.8F, -2.35F},
        0.38F));
    obstaculos_.push_back(std::make_unique<BarreraEstatica>(
        Posicion{21.75F, -2.55F},
        0.38F));

    // Fase 4: pared vertical rosada para rebotar desde checkpoint 3 hacia checkpoint 4.
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{18.4F, 0.2F},
        Posicion{-1.0F, 0.0F},
        0.86F));

    // Fase 5: rebote consecutivo para salir de checkpoint 4 hacia checkpoint 5.
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{18.7F, -1.35F},
        Posicion{0.13F, 0.99F},
        0.78F));
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{21.0F, -3.1F},
        Posicion{0.25F, 0.97F},
        0.78F));

    for (const auto& obstaculo : obstaculos_) {
        nivel->agregarObstaculo(*obstaculo);
    }

    // Fase 2: dos drones verticales despues de checkpoint 1 hacia checkpoint 2.
    drones_.push_back(std::make_unique<Dron>(Posicion{6.65F, -0.35F}, 1.3F));
    drones_.back()->definirRutaVigilancia({
        Posicion{6.65F, 1.25F},
        Posicion{6.65F, -2.35F}
    });
    drones_.push_back(std::make_unique<Dron>(Posicion{8.05F, -2.35F}, 1.35F));
    drones_.back()->definirRutaVigilancia({
        Posicion{8.05F, -2.35F},
        Posicion{8.05F, 1.05F}
    });

    // Fase 3: dron horizontal en la subida desde checkpoint 2.
    drones_.push_back(std::make_unique<Dron>(Posicion{11.2F, -0.45F}, 1.35F));
    drones_.back()->definirRutaVigilancia({
        Posicion{10.55F, -0.45F},
        Posicion{12.55F, -0.45F}
    });

    // Fase 6: defensor rojo final, restringido al acceso del nodo central.
    drones_.push_back(std::make_unique<Dron>(Posicion{28.25F, -2.55F}, 2.05F));
    drones_.back()->definirRutaVigilancia({
        Posicion{27.65F, -1.55F},
        Posicion{28.95F, -3.05F},
        Posicion{27.65F, -3.05F},
        Posicion{28.95F, -1.55F}
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
