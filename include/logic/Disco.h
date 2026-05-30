#pragma once

#include "logic/Posicion.h"

class Disco {
public:
    Disco() = default;
    explicit Disco(Posicion posicion);
    virtual ~Disco();

    const Posicion& posicion() const noexcept;
    float velocidad() const noexcept;
    const Posicion& direccion() const noexcept;
    bool estaActivo() const noexcept;
    bool estaEnMovimiento() const noexcept;

    void lanzarDesde(const Posicion& origen, const Posicion& direccion, float velocidad) noexcept;
    void actualizar(float segundos) noexcept;
    void detener() noexcept;
    void reiniciar() noexcept;
    void reiniciarEn(const Posicion& posicion) noexcept;
    bool colisionaCon(const Posicion& punto, float tolerancia) const noexcept;

protected:
    Posicion posicion_;
    Posicion posicionInicial_;
    Posicion direccion_;
    float velocidad_{0.0F};
    bool activo_{false};
    bool enMovimiento_{false};
};
