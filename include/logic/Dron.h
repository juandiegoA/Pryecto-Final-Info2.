#pragma once

#include "logic/Posicion.h"

class Dron {
public:
    Dron() = default;
    explicit Dron(Posicion posicion);

    const Posicion& posicion() const noexcept;

private:
    Posicion posicion_;
};

