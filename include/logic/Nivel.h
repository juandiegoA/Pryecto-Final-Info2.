#pragma once

#include <chrono>
#include <string>

// Contrato base para cualquier escenario jugable.
class Nivel {
public:
    virtual ~Nivel();

    virtual std::string nombre() const = 0;
    virtual void actualizar() = 0;
    virtual void actualizar(std::chrono::milliseconds intervalo);
};
