#pragma once

#include "logic/Checkpoint.h"

class NodoCentralEnergia final : public Checkpoint {
public:
    explicit NodoCentralEnergia(int energia = 100);
    NodoCentralEnergia(
        std::string id,
        Posicion posicion,
        std::chrono::milliseconds tiempoMaximo,
        int energia = 100);

    int energia() const noexcept;
    void marcarFinNivel() noexcept;
    bool nivelFinalizado() const noexcept;

private:
    int energia_;
    bool nivelFinalizado_{false};
};
