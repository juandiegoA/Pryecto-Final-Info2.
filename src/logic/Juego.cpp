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
    jugador_ = Jugador{Posicion{0.0F, 0.0F}};
    discoJugador_ = Disco{Posicion{0.0F, 0.0F}};

    auto nivel = std::make_unique<NivelRutaTransmision>(jugador_, discoJugador_);

    // Fases 2, 3, 4, 5 y 6: cinco nodos obligatorios antes del nodo central.
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-1",
        Posicion{5.2F, 0.0F},
        5400ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-2",
        Posicion{9.55F, -2.65F},
        6200ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-3",
        Posicion{12.75F, 1.35F},
        6500ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-4",
        Posicion{16.15F, -1.95F},
        7200ms));
    checkpoints_.push_back(std::make_unique<Checkpoint>(
        "checkpoint-5",
        Posicion{20.05F, 0.15F},
        7000ms));

    for (const auto& checkpoint : checkpoints_) {
        nivel->agregarCheckpoint(*checkpoint);
    }

    metaFinal_ = std::make_unique<NodoCentralEnergia>(
        "nodo-central",
        Posicion{23.25F, 0.0F},
        6500ms);
    nivel->establecerMetaFinal(*metaFinal_);

    // Fase 1: compuerta de entrada, dos paredes enfrentadas que abren y cierran.
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{2.55F, 1.25F},
        Posicion{2.55F, 0.18F},
        0.95F,
        0.5F));
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{2.55F, -1.25F},
        Posicion{2.55F, -0.18F},
        0.95F,
        0.5F));

    // Fase 4: transicion media con una pared movil y una barrera temporizada.
    obstaculos_.push_back(std::make_unique<BarreraMovil>(
        Posicion{11.3F, -0.75F},
        Posicion{11.3F, 1.1F},
        0.65F,
        0.43F));
    obstaculos_.push_back(std::make_unique<BarreraTemporizada>(
        Posicion{12.05F, 0.35F},
        0.45F,
        1150ms,
        850ms));

    // Fase 6: bloqueo previo al defensor final.
    obstaculos_.push_back(std::make_unique<BarreraTemporizada>(
        Posicion{18.65F, -0.55F},
        0.5F,
        800ms,
        900ms));

    // Fase 5: superficies rosadas usadas para redirigir el disco hacia la salida.
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{14.8F, -2.65F},
        Posicion{0.8F, 1.0F},
        0.78F));
    obstaculos_.push_back(std::make_unique<SuperficieRebote>(
        Posicion{17.05F, -1.05F},
        Posicion{-0.75F, 1.0F},
        0.78F));

    for (const auto& obstaculo : obstaculos_) {
        nivel->agregarObstaculo(*obstaculo);
    }

    // Fase 2: dos drones verticales que enmarcan el primer checkpoint.
    drones_.push_back(std::make_unique<Dron>(Posicion{4.65F, -1.15F}, 1.3F));
    drones_.back()->definirRutaVigilancia({
        Posicion{4.65F, -1.15F},
        Posicion{4.65F, 1.15F}
    });
    drones_.push_back(std::make_unique<Dron>(Posicion{5.75F, 1.15F}, 1.3F));
    drones_.back()->definirRutaVigilancia({
        Posicion{5.75F, 1.15F},
        Posicion{5.75F, -1.15F}
    });

    // Fase 3: descenso con drones asincronicos, uno sube mientras el otro baja.
    drones_.push_back(std::make_unique<Dron>(Posicion{7.45F, -0.9F}, 1.25F));
    drones_.back()->definirRutaVigilancia({
        Posicion{7.45F, -0.9F},
        Posicion{7.45F, -3.0F}
    });
    drones_.push_back(std::make_unique<Dron>(Posicion{8.6F, -3.0F}, 1.25F));
    drones_.back()->definirRutaVigilancia({
        Posicion{8.6F, -3.0F},
        Posicion{8.6F, -0.9F}
    });

    // Fase 4: un dron de patrulla corta en la transicion media.
    drones_.push_back(std::make_unique<Dron>(Posicion{13.85F, 0.25F}, 1.1F));
    drones_.back()->definirRutaVigilancia({
        Posicion{13.85F, 0.25F},
        Posicion{14.8F, 1.45F},
        Posicion{12.95F, 1.9F}
    });

    // Fase 7: defensor rojo final, restringido al acceso del nodo central.
    drones_.push_back(std::make_unique<Dron>(Posicion{21.85F, 0.0F}, 2.05F));
    drones_.back()->definirRutaVigilancia({
        Posicion{21.35F, 0.95F},
        Posicion{22.35F, -0.95F},
        Posicion{21.35F, -0.95F},
        Posicion{22.35F, 0.95F}
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
