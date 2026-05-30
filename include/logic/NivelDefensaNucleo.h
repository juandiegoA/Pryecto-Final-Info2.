#pragma once

#include "logic/DiscoEnemigo.h"
#include "logic/Nivel.h"
#include "logic/Temporizador.h"

#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

class Jugador;

class NivelDefensaNucleo final : public Nivel {
public:
    NivelDefensaNucleo() = default;
    NivelDefensaNucleo(Jugador& jugador, std::chrono::milliseconds tiempoObjetivo);

    std::string nombre() const override;
    void actualizar() override;
    void actualizar(std::chrono::milliseconds intervalo) override;

    void configurarJugador(Jugador& jugador) noexcept;
    void configurarTiempoObjetivo(std::chrono::milliseconds tiempoObjetivo);
    void iniciar();
    void agregarDiscoEnemigo(DiscoEnemigo& disco);
    DiscoEnemigo& generarDiscoEnemigo(Posicion posicion);
    bool destruirDiscoEnemigo(std::size_t indice) noexcept;
    bool verificarImpactoJugador() const noexcept;

    const std::vector<DiscoEnemigo*>& obtenerDiscosEnemigos() const noexcept;
    std::size_t discosActivos() const noexcept;
    std::chrono::milliseconds tiempoTranscurrido() const noexcept;
    std::chrono::milliseconds tiempoRestante() const noexcept;
    bool estaEnCurso() const noexcept;
    bool estaFinalizado() const override;
    bool victoria() const override;
    bool derrota() const override;

private:
    void moverDiscos(float segundos) noexcept;
    void generarDiscosSiHaceFalta(std::chrono::milliseconds intervalo);
    void declararVictoria() noexcept;
    void declararDerrota() noexcept;

    std::vector<DiscoEnemigo*> discosEnemigos;
    std::vector<std::unique_ptr<DiscoEnemigo>> discosGenerados_;
    Jugador* jugador_{nullptr};
    Temporizador temporizadorNivel_{std::chrono::milliseconds{30000}};
    std::chrono::milliseconds tiempoDesdeUltimoDisparo_{0};
    std::chrono::milliseconds intervaloDisparo_{2200};
    float velocidadDiscoEnemigo_{2.0F};
    float toleranciaImpacto_{0.5F};
    std::size_t maxDiscosActivos_{5};
    std::size_t siguienteCarril_{0};
    bool enCurso_{false};
    bool victoria_{false};
    bool derrota_{false};
};
