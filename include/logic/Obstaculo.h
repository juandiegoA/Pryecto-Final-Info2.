#pragma once

class Disco;

// Punto de extension para colisiones y barreras del escenario.
class Obstaculo {
public:
    virtual ~Obstaculo();

    virtual bool estaActivo() const noexcept = 0;
    virtual void actualizar(float segundos) noexcept;
    virtual bool bloqueaAl(const Disco& disco) const noexcept = 0;
};
