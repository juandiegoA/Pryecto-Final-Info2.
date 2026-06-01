#include "logic/SuperficieRebote.h"

#include "logic/Disco.h"

SuperficieRebote::SuperficieRebote(
    Posicion posicion,
    Posicion normal,
    float tolerancia)
    : posicion_(posicion),
      normal_(normal),
      tolerancia_(tolerancia >= 0.0F ? tolerancia : 0.0F) {}

bool SuperficieRebote::estaActivo() const noexcept {
    return true;
}

bool SuperficieRebote::bloqueaAl(const Disco& disco) const noexcept {
    return false;
}

bool SuperficieRebote::aplicarRebote(Disco& disco) noexcept {
    const bool enContacto = disco.cruzaPor(posicion_, tolerancia_);
    if (!enContacto) {
        discoEnContacto_ = false;
        return false;
    }

    if (discoEnContacto_) {
        return false;
    }

    disco.reflejar(normal_);
    discoEnContacto_ = true;
    return true;
}

const Posicion& SuperficieRebote::posicion() const noexcept {
    return posicion_;
}

const Posicion& SuperficieRebote::normal() const noexcept {
    return normal_;
}

float SuperficieRebote::tolerancia() const noexcept {
    return tolerancia_;
}
