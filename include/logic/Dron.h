#pragma once

#include "logic/Posicion.h"

#include <cstddef>
#include <vector>

class Disco;
class Jugador;

class Dron {
public:
    Dron() = default;
    explicit Dron(Posicion posicion);
    Dron(Posicion posicion, float velocidad);

    const Posicion& posicion() const noexcept;
    float velocidad() const noexcept;
    bool estaActivo() const noexcept;
    const std::vector<Posicion>& rutaVigilancia() const noexcept;

    void establecerVelocidad(float velocidad) noexcept;
    void activar() noexcept;
    void desactivar() noexcept;
    void vigilarZona(const Posicion& objetivo) noexcept;
    void definirRutaVigilancia(std::vector<Posicion> ruta);
    void intentarInterceptar(const Disco& disco, const Jugador& jugador) noexcept;
    void actualizar(float segundos) noexcept;
    bool bloqueaDisco(const Disco& disco, float tolerancia) const noexcept;

private:
    void moverHaciaObjetivo(float segundos) noexcept;

    Posicion posicion_;
    Posicion objetivo_;
    std::vector<Posicion> rutaVigilancia_;
    std::size_t indiceRuta_{0};
    float velocidad_{1.0F};
    bool activo_{true};
    bool tieneObjetivo_{false};
    bool siguiendoRuta_{false};
};
