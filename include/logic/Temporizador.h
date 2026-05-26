#pragma once

#include <chrono>

class Temporizador {
public:
    explicit Temporizador(std::chrono::milliseconds duracion = std::chrono::milliseconds{0});

    void iniciar() noexcept;
    void actualizar(std::chrono::milliseconds intervalo) noexcept;
    void reiniciar() noexcept;
    void detener() noexcept;
    bool estaActivo() const noexcept;
    bool estaAgotado() const noexcept;
    std::chrono::milliseconds duracion() const noexcept;
    std::chrono::milliseconds tiempoTranscurrido() const noexcept;
    std::chrono::milliseconds tiempoRestante() const noexcept;

private:
    std::chrono::milliseconds duracion_;
    std::chrono::milliseconds transcurrido_{0};
    bool activo_{false};
};
