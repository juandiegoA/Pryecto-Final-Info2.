#include "logic/NivelRutaTransmision.h"

#include "logic/Checkpoint.h"
#include "logic/Disco.h"
#include "logic/Jugador.h"
#include "logic/NodoCentralEnergia.h"
#include "logic/Obstaculo.h"

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

    disco_->actualizar(segundos);
    temporizadorCheckpoint_.actualizar(intervalo);

    if (verificarColisionConObstaculos()) {
        reiniciarDesdeUltimoCheckpoint();
        return;
    }

    Checkpoint* objetivo = objetivoActual();
    if (objetivo != nullptr && verificarLlegada(*objetivo, toleranciaCheckpoint_)) {
        alcanzarObjetivo(*objetivo);
        return;
    }

    if (temporizadorCheckpoint_.duracion().count() > 0
        && temporizadorCheckpoint_.estaAgotado()) {
        reiniciarDesdeUltimoCheckpoint();
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

void NivelRutaTransmision::agregarObstaculo(Obstaculo& obstaculo) {
    obstaculos.push_back(&obstaculo);
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

const std::vector<Obstaculo*>& NivelRutaTransmision::obtenerObstaculos() const noexcept {
    return obstaculos;
}

const Checkpoint* NivelRutaTransmision::checkpointActual() const noexcept {
    return checkpointActual_;
}

const NodoCentralEnergia* NivelRutaTransmision::metaFinal() const noexcept {
    return metaFinal_;
}

bool NivelRutaTransmision::estaFinalizado() const noexcept {
    return metaFinal_ != nullptr && metaFinal_->nivelFinalizado();
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

void NivelRutaTransmision::reiniciarDesdeUltimoCheckpoint() {
    if (jugador_ == nullptr || disco_ == nullptr) {
        return;
    }

    if (checkpointActual_ != nullptr) {
        jugador_->teletransportarA(checkpointActual_->posicion());
    } else {
        jugador_->reiniciar();
    }

    disco_->reiniciar();
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
    disco_->reiniciar();

    if (&checkpoint == metaFinal_) {
        metaFinal_->marcarFinNivel();
        temporizadorCheckpoint_.detener();
        return;
    }

    ++indiceObjetivo_;
    iniciarTiempoObjetivo();
}
