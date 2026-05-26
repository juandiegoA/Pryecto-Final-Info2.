#pragma once

class Posicion {
public:
    Posicion() = default;
    Posicion(float x, float y);

    float x() const noexcept;
    float y() const noexcept;
    float distanciaA(const Posicion& otra) const noexcept;
    void establecer(float x, float y) noexcept;

private:
    float x_{0.0F};
    float y_{0.0F};
};
