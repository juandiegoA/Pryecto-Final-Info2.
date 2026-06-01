#pragma once

#include "logic/Obstaculo.h"
#include "logic/Posicion.h"

class SuperficieRebote final : public Obstaculo {
public:
    SuperficieRebote() = default;
    SuperficieRebote(Posicion posicion, Posicion normal, float tolerancia);

    bool estaActivo() const noexcept override;
    bool bloqueaAl(const Disco& disco) const noexcept override;
    bool aplicarRebote(Disco& disco) noexcept;

    const Posicion& posicion() const noexcept;
    const Posicion& normal() const noexcept;
    float tolerancia() const noexcept;

private:
    Posicion posicion_;
    Posicion normal_{0.0F, 1.0F};
    float tolerancia_{0.5F};
    bool discoEnContacto_{false};
};
