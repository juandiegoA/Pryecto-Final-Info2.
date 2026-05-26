#include "logic/Posicion.h"

#include <cmath>

Posicion::Posicion(float x, float y) : x_(x), y_(y) {}

float Posicion::x() const noexcept {
    return x_;
}

float Posicion::y() const noexcept {
    return y_;
}

float Posicion::distanciaA(const Posicion& otra) const noexcept {
    return std::hypot(otra.x_ - x_, otra.y_ - y_);
}

void Posicion::establecer(float x, float y) noexcept {
    x_ = x;
    y_ = y;
}
