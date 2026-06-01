#include "logic/BarreraMovil.h"

#include "logic/Disco.h"

#include <algorithm>

BarreraMovil::BarreraMovil(
    Posicion inicio,
    Posicion fin,
    float velocidad,
    float tolerancia)
    : inicio_(inicio),
      fin_(fin),
      posicion_(inicio),
      velocidad_(std::max(velocidad, 0.0F)),
      tolerancia_(std::max(tolerancia, 0.0F)) {}

bool BarreraMovil::estaActivo() const noexcept {
    return true;
}

void BarreraMovil::actualizar(float segundos) noexcept {
    if (segundos <= 0.0F || velocidad_ <= 0.0F) {
        return;
    }

    const Posicion& destino = haciaFin_ ? fin_ : inicio_;
    const float distancia = posicion_.distanciaA(destino);
    if (distancia <= 0.01F) {
        haciaFin_ = !haciaFin_;
        return;
    }

    const float avance = std::min(velocidad_ * segundos, distancia);
    const float factor = avance / distancia;
    posicion_.establecer(
        posicion_.x() + (destino.x() - posicion_.x()) * factor,
        posicion_.y() + (destino.y() - posicion_.y()) * factor);
}

bool BarreraMovil::bloqueaAl(const Disco& disco) const noexcept {
    return disco.cruzaPor(posicion_, tolerancia_);
}

const Posicion& BarreraMovil::posicion() const noexcept {
    return posicion_;
}
