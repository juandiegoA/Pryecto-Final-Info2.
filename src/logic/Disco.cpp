#include "logic/Disco.h"

#include <cmath>

Disco::Disco(Posicion posicion) : posicion_(posicion), posicionInicial_(posicion) {}

Disco::~Disco() = default;

const Posicion& Disco::posicion() const noexcept {
    return posicion_;
}

float Disco::velocidad() const noexcept {
    return velocidad_;
}

const Posicion& Disco::direccion() const noexcept {
    return direccion_;
}

bool Disco::estaActivo() const noexcept {
    return activo_;
}

bool Disco::estaEnMovimiento() const noexcept {
    return enMovimiento_;
}

void Disco::lanzarDesde(
    const Posicion& origen,
    const Posicion& direccion,
    float velocidad) noexcept {
    const float magnitud = std::hypot(direccion.x(), direccion.y());

    posicion_ = origen;
    direccion_ = Posicion{};
    velocidad_ = 0.0F;
    activo_ = true;
    enMovimiento_ = false;

    if (magnitud <= 0.0F || velocidad <= 0.0F) {
        return;
    }

    direccion_.establecer(direccion.x() / magnitud, direccion.y() / magnitud);
    velocidad_ = velocidad;
    enMovimiento_ = true;
}

void Disco::actualizar(float segundos) noexcept {
    if (!activo_ || !enMovimiento_ || segundos <= 0.0F) {
        return;
    }

    posicion_.establecer(
        posicion_.x() + direccion_.x() * velocidad_ * segundos,
        posicion_.y() + direccion_.y() * velocidad_ * segundos);
}

void Disco::detener() noexcept {
    enMovimiento_ = false;
    velocidad_ = 0.0F;
}

void Disco::reiniciar() noexcept {
    posicion_ = posicionInicial_;
    direccion_ = Posicion{};
    velocidad_ = 0.0F;
    activo_ = false;
    enMovimiento_ = false;
}

bool Disco::colisionaCon(const Posicion& punto, float tolerancia) const noexcept {
    return activo_ && tolerancia >= 0.0F && posicion_.distanciaA(punto) <= tolerancia;
}
