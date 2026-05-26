#pragma once

#include "logic/Posicion.h"

#include <chrono>
#include <string>

class Disco;

class Checkpoint {
public:
    Checkpoint() = default;
    explicit Checkpoint(Posicion posicion);
    Checkpoint(std::string id, Posicion posicion, std::chrono::milliseconds tiempoMaximo);
    virtual ~Checkpoint() = default;

    const std::string& id() const noexcept;
    const Posicion& posicion() const noexcept;
    std::chrono::milliseconds tiempoMaximo() const noexcept;
    bool estaActivado() const noexcept;
    void activar() noexcept;
    void desactivar() noexcept;
    bool verificarLlegada(const Disco& disco, float tolerancia) const noexcept;

private:
    std::string id_;
    Posicion posicion_;
    std::chrono::milliseconds tiempoMaximo_{0};
    bool activado_{false};
};
