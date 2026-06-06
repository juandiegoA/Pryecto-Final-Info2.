#include "logic/NivelRutaTransmision.h"

#include "logic/Checkpoint.h"
#include "logic/Disco.h"
#include "logic/Dron.h"
#include "logic/Jugador.h"
#include "logic/NodoCentralEnergia.h"
#include "logic/Obstaculo.h"
#include "logic/SuperficieRebote.h"

#include <utility>

NivelRutaTransmision::NivelRutaTransmision(Jugador& jugador, Disco& disco)
    : jugador_(&jugador), disco_(&disco) {}

std::string NivelRutaTransmision::nombre() const {
    return "Ruta de Transmision";
}

void NivelRutaTransmision::actualizar() {
    actualizar(std::chrono::milliseconds{0});
}

void NivelRutaTransmision::actualizar(std::chrono::milliseconds intervalo) {
    if (estaFinalizado() || jugador_ == nullptr || disco_ == nullptr) {
        return;
    }

    const float segundos = static_cast<float>(intervalo.count()) / 1000.0F;
    for (Obstaculo* obstaculo : obstaculos) {
        if (obstaculo != nullptr) {
            obstaculo->actualizar(segundos);
        }
    }

    actualizarAgenteInteligente();

    for (Dron* dron : drones) {
        if (dron != nullptr) {
            dron->actualizar(segundos);
        }
    }

    disco_->actualizar(segundos);
    aplicarRebotes();
    temporizadorCheckpoint_.actualizar(intervalo);

    if (verificarColisionConObstaculos() || verificarColisionConDrones()) {
        reiniciarDesdeUltimoCheckpoint();
        return;
    }

    if (temporizadorCheckpoint_.duracion().count() > 0
        && temporizadorCheckpoint_.estaAgotado()) {
        reiniciarDesdeUltimoCheckpoint();
        return;
    }

    Checkpoint* objetivo = objetivoActual();
    if (objetivo != nullptr && verificarLlegada(*objetivo, toleranciaCheckpoint_)) {
        alcanzarObjetivo(*objetivo);
    }
}

void NivelRutaTransmision::configurarActores(Jugador& jugador, Disco& disco) noexcept {
    jugador_ = &jugador;
    disco_ = &disco;
    iniciarTiempoObjetivo();
}

void NivelRutaTransmision::agregarCheckpoint(Checkpoint& checkpoint) {
    checkpoints.push_back(&checkpoint);
    if (checkpoints.size() == 1 && indiceObjetivo_ == 0) {
        iniciarTiempoObjetivo();
    }
}

void NivelRutaTransmision::agregarDron(Dron& dron) {
    drones.push_back(&dron);
}

void NivelRutaTransmision::agregarObstaculo(Obstaculo& obstaculo) {
    obstaculos.push_back(&obstaculo);
}

void NivelRutaTransmision::agregarTramo(TramoRutaTransmision tramo) {
    tramos_.push_back(std::move(tramo));
}

void NivelRutaTransmision::establecerMetaFinal(NodoCentralEnergia& meta) noexcept {
    metaFinal_ = &meta;
    if (indiceObjetivo_ >= checkpoints.size()) {
        iniciarTiempoObjetivo();
    }
}

const std::vector<Checkpoint*>& NivelRutaTransmision::obtenerCheckpoints() const noexcept {
    return checkpoints;
}

const std::vector<Dron*>& NivelRutaTransmision::obtenerDrones() const noexcept {
    return drones;
}

const std::vector<Obstaculo*>& NivelRutaTransmision::obtenerObstaculos() const noexcept {
    return obstaculos;
}

const std::vector<TramoRutaTransmision>& NivelRutaTransmision::obtenerTramos() const noexcept {
    return tramos_;
}

const Checkpoint* NivelRutaTransmision::checkpointActual() const noexcept {
    return checkpointActual_;
}

const Checkpoint* NivelRutaTransmision::objetivoActualCheckpoint() const noexcept {
    return objetivoActual();
}

const NodoCentralEnergia* NivelRutaTransmision::metaFinal() const noexcept {
    return metaFinal_;
}

std::chrono::milliseconds NivelRutaTransmision::tiempoRestanteCheckpoint() const noexcept {
    return temporizadorCheckpoint_.tiempoRestante();
}

bool NivelRutaTransmision::estaFinalizado() const {
    return metaFinal_ != nullptr && metaFinal_->nivelFinalizado();
}

bool NivelRutaTransmision::victoria() const {
    return estaFinalizado();
}

bool NivelRutaTransmision::verificarLlegada(
    const Checkpoint& checkpoint,
    float tolerancia) const noexcept {
    return disco_ != nullptr && checkpoint.verificarLlegada(*disco_, tolerancia);
}

bool NivelRutaTransmision::verificarColisionConObstaculos() const noexcept {
    if (disco_ == nullptr) {
        return false;
    }

    for (const Obstaculo* obstaculo : obstaculos) {
        if (obstaculo != nullptr && obstaculo->bloqueaAl(*disco_)) {
            return true;
        }
    }

    return false;
}

bool NivelRutaTransmision::verificarColisionConDrones() const noexcept {
    if (disco_ == nullptr) {
        return false;
    }

    for (const Dron* dron : drones) {
        if (dron != nullptr && dron->bloqueaDisco(*disco_, toleranciaDron_)) {
            return true;
        }
    }

    return false;
}

void NivelRutaTransmision::reiniciarDesdeUltimoCheckpoint() {
    if (jugador_ == nullptr || disco_ == nullptr) {
        return;
    }

    if (indiceObjetivo_ < checkpoints.size()) {
        agente_.registrarRutaFrecuente(static_cast<int>(indiceObjetivo_));
    }

    Posicion posicionReinicio = jugador_->posicion();
    if (checkpointActual_ != nullptr) {
        posicionReinicio = checkpointActual_->posicion();
        jugador_->teletransportarA(posicionReinicio);
    } else {
        jugador_->reiniciar();
        posicionReinicio = jugador_->posicion();
    }

    disco_->reiniciarEn(posicionReinicio);
    iniciarTiempoObjetivo();
}

Checkpoint* NivelRutaTransmision::objetivoActual() const noexcept {
    if (indiceObjetivo_ < checkpoints.size()) {
        return checkpoints[indiceObjetivo_];
    }

    return metaFinal_;
}

void NivelRutaTransmision::iniciarTiempoObjetivo() {
    Checkpoint* objetivo = objetivoActual();
    const auto limite = objetivo != nullptr
        ? objetivo->tiempoMaximo()
        : std::chrono::milliseconds{0};

    temporizadorCheckpoint_ = Temporizador{limite};
    temporizadorCheckpoint_.iniciar();
}

void NivelRutaTransmision::alcanzarObjetivo(Checkpoint& checkpoint) {
    checkpoint.activar();
    checkpointActual_ = &checkpoint;
    jugador_->teletransportarA(checkpoint.posicion());
    if (indiceObjetivo_ < checkpoints.size()) {
        agente_.registrarRutaFrecuente(static_cast<int>(indiceObjetivo_));
    }

    if (metaFinal_ != nullptr && &checkpoint == static_cast<Checkpoint*>(metaFinal_)) {
        metaFinal_->marcarFinNivel();
        temporizadorCheckpoint_.detener();
        disco_->detener();
        return;
    }

    disco_->reiniciarEn(checkpoint.posicion());
    ++indiceObjetivo_;
    iniciarTiempoObjetivo();
}

void NivelRutaTransmision::aplicarRebotes() {
    if (disco_ == nullptr) {
        return;
    }

    for (Obstaculo* obstaculo : obstaculos) {
        auto* superficie = dynamic_cast<SuperficieRebote*>(obstaculo);
        if (superficie != nullptr) {
            superficie->aplicarRebote(*disco_);
        }
    }
}

void NivelRutaTransmision::actualizarAgenteInteligente() {
    if (jugador_ == nullptr || disco_ == nullptr || checkpoints.empty() || drones.empty()) {
        return;
    }

    agente_.percibir(*jugador_, *disco_, checkpoints, objetivoActual());

    if (indiceObjetivo_ + 1U < checkpoints.size()) {
        return;
    }

    Dron* dron = dronDefensorFinal();
    if (dron != nullptr) {
        agente_.instruirDefensorFinal(*dron, zonaDefensaFinal());
    }
}

Dron* NivelRutaTransmision::dronDefensorFinal() noexcept {
    if (drones.empty()) {
        return nullptr;
    }

    Dron* defensor = drones.back();
    return defensor != nullptr && defensor->estaActivo() ? defensor : nullptr;
}

Posicion NivelRutaTransmision::zonaDefensaFinal() const noexcept {
    if (metaFinal_ != nullptr) {
        return Posicion{metaFinal_->posicion().x() - 3.8F, metaFinal_->posicion().y()};
    }

    return Posicion{35.2F, -2.8F};
}
