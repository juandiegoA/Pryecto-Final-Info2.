#pragma once

#include "logic/DificultadDefensa.h"
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

    void configurarDificultad(DificultadDefensa dificultad);
    void configurarJugador(Jugador& jugador) noexcept;
    void configurarTiempoObjetivo(std::chrono::milliseconds tiempoObjetivo);
    void iniciar();
    void agregarDiscoEnemigo(DiscoEnemigo& disco);
    DiscoEnemigo& generarDiscoEnemigo(Posicion posicion);
    void dispararDefensa(const Posicion& objetivo) noexcept;
    bool destruirDiscoEnemigo(std::size_t indice) noexcept;
    bool verificarImpactoJugador() const noexcept;

    const std::vector<DiscoEnemigo*>& obtenerDiscosEnemigos() const noexcept;
    DiscoEnemigo& discoEnemigoEn(std::size_t indice);
    const DiscoEnemigo& discoEnemigoEn(std::size_t indice) const;
    const Posicion& posicionProyectilDefensor() const noexcept;
    bool proyectilDefensorActivo() const noexcept;
    DificultadDefensa dificultad() const noexcept;
    std::size_t discosActivos() const noexcept;
    std::chrono::milliseconds tiempoTranscurrido() const noexcept;
    std::chrono::milliseconds tiempoRestante() const noexcept;
    bool estaEnCurso() const noexcept;
    bool estaFinalizado() const override;
    bool victoria() const override;
    bool derrota() const override;

private:
    struct ConfiguracionDificultad {
        std::vector<Posicion> carriles;
        std::chrono::milliseconds intervaloDisparoEnemigo{2200};
        std::chrono::milliseconds intervaloProyectilDefensor{300};
        float velocidadDiscoEnemigo{2.0F};
        float velocidadProyectilDefensor{8.0F};
        float toleranciaIntercepcion{0.45F};
        std::size_t maxDiscosActivos{5};
    };

    static ConfiguracionDificultad crearConfiguracion(DificultadDefensa dificultad);
    void moverDiscos(float segundos) noexcept;
    void actualizarProyectilDefensor(float segundos) noexcept;
    void verificarIntercepcionDefensiva() noexcept;
    void generarDiscosSiHaceFalta(std::chrono::milliseconds intervalo);
    void desactivarDiscosEnemigos() noexcept;
    void declararVictoria() noexcept;
    void declararDerrota() noexcept;

    std::vector<DiscoEnemigo*> discosEnemigos;
    std::vector<std::unique_ptr<DiscoEnemigo>> discosGenerados_;
    Jugador* jugador_{nullptr};
    Temporizador temporizadorNivel_{std::chrono::milliseconds{30000}};
    std::chrono::milliseconds tiempoDesdeUltimoDisparo_{0};
    std::chrono::milliseconds tiempoDesdeUltimoDisparoDefensor_{0};
    std::chrono::milliseconds intervaloDisparo_{2200};
    std::chrono::milliseconds intervaloProyectilDefensor_{300};
    Posicion posicionProyectilDefensor_;
    Posicion objetivoProyectilDefensor_;
    Posicion direccionProyectilDefensor_;
    float velocidadDiscoEnemigo_{2.0F};
    float velocidadProyectilDefensor_{8.0F};
    float toleranciaImpacto_{0.5F};
    float toleranciaIntercepcion_{0.45F};
    std::size_t maxDiscosActivos_{5};
    std::size_t siguienteCarril_{0};
    std::vector<Posicion> carrilesAparicion_;
    DificultadDefensa dificultad_{DificultadDefensa::Medio};
    bool proyectilDefensorActivo_{false};
    bool enCurso_{false};
    bool victoria_{false};
    bool derrota_{false};
};
