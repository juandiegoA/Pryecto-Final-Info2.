#pragma once

#include "logic/Posicion.h"

#include <cstddef>
#include <vector>

class Checkpoint;
class Disco;
class Dron;
class Jugador;

class AgenteInteligente {
public:
    AgenteInteligente() = default;
    explicit AgenteInteligente(Posicion posicion);

    const Posicion& posicion() const noexcept;
    const Posicion& objetivoVigilancia() const noexcept;
    const std::vector<int>& rutasFrecuentes() const noexcept;

    void percibir(
        const Jugador& jugador,
        const Disco& disco,
        const std::vector<Checkpoint*>& checkpoints,
        const Checkpoint* objetivoActual);
    std::size_t estimarIndiceRutaProbable() const noexcept;
    void instruir(Dron& dron) const noexcept;
    void instruirDefensorFinal(Dron& dron, const Posicion& zonaDefensa) const noexcept;
    void registrarRutaFrecuente(int indiceRuta);

private:
    int elegirRutaFrecuente(std::size_t cantidadRutas) const noexcept;

    Posicion posicion_;
    Posicion posicionJugador_;
    Posicion posicionDisco_;
    Posicion direccionDisco_;
    Posicion objetivoVigilancia_;
    std::vector<int> rutasFrecuentes_;
    std::size_t indiceRutaProbable_{0};
    int repeticionesRutaProbable_{0};
    float velocidadDisco_{0.0F};
    bool discoActivo_{false};
    bool discoEnMovimiento_{false};
    bool tieneObjetivo_{false};
};
