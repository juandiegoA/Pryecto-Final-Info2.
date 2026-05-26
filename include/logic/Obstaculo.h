#pragma once

// Punto de extension para colisiones y barreras del escenario.
class Obstaculo {
public:
    virtual ~Obstaculo();

    virtual bool estaActivo() const noexcept = 0;
};

