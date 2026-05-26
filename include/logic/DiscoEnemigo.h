#pragma once

#include "logic/Disco.h"

class DiscoEnemigo final : public Disco {
public:
    DiscoEnemigo() = default;
    explicit DiscoEnemigo(Posicion posicion);
};

