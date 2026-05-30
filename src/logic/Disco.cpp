#include "logic/Disco.h"

#include <algorithm>
#include <cmath>

Disco::Disco(Posicion posicion)
    : posicion_(posicion), posicionAnterior_(posicion), posicionInicial_(posicion) {}

Disco::~Disco() = default;

const Posicion& Disco::posicion() const noexcept {
    return posicion_;
}

const Posicion& Disco::posicionAnterior() const noexcept {
    return posicionAnterior_;
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
    posicionAnterior_ = origen;
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

    posicionAnterior_ = posicion_;
    posicion_.establecer(
        posicion_.x() + direccion_.x() * velocidad_ * segundos,
        posicion_.y() + direccion_.y() * velocidad_ * segundos);
}

void Disco::detener() noexcept {
    enMovimiento_ = false;
    velocidad_ = 0.0F;
}

void Disco::reiniciar() noexcept {
    reiniciarEn(posicionInicial_);
}

void Disco::reiniciarEn(const Posicion& posicion) noexcept {
    posicionInicial_ = posicion;
    posicion_ = posicionInicial_;
    posicionAnterior_ = posicionInicial_;
    direccion_ = Posicion{};
    velocidad_ = 0.0F;
    activo_ = false;
    enMovimiento_ = false;
}

bool Disco::colisionaCon(const Posicion& punto, float tolerancia) const noexcept {
    return activo_ && tolerancia >= 0.0F && posicion_.distanciaA(punto) <= tolerancia;
}

bool Disco::cruzaPor(const Posicion& punto, float tolerancia) const noexcept {
    if (!activo_ || tolerancia < 0.0F) {
        return false;
    }

    const float dx = posicion_.x() - posicionAnterior_.x();
    const float dy = posicion_.y() - posicionAnterior_.y();
    const float longitudCuadrada = dx * dx + dy * dy;

    if (longitudCuadrada <= 0.0F) {
        return colisionaCon(punto, tolerancia);
    }

    const float t = std::clamp(
        ((punto.x() - posicionAnterior_.x()) * dx
            + (punto.y() - posicionAnterior_.y()) * dy) / longitudCuadrada,
        0.0F,
        1.0F);
    const Posicion puntoMasCercano{
        posicionAnterior_.x() + dx * t,
        posicionAnterior_.y() + dy * t};

    return puntoMasCercano.distanciaA(punto) <= tolerancia;
}
