#include "logic/NodoCentralEnergia.h"

#include <utility>

NodoCentralEnergia::NodoCentralEnergia(int energia) : energia_(energia) {}

NodoCentralEnergia::NodoCentralEnergia(
    std::string id,
    Posicion posicion,
    std::chrono::milliseconds tiempoMaximo,
    int energia)
    : Checkpoint(std::move(id), posicion, tiempoMaximo), energia_(energia) {}

int NodoCentralEnergia::energia() const noexcept {
    return energia_;
}

void NodoCentralEnergia::marcarFinNivel() noexcept {
    activar();
    nivelFinalizado_ = true;
}

bool NodoCentralEnergia::nivelFinalizado() const noexcept {
    return nivelFinalizado_;
}
