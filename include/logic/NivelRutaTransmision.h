#pragma once

#include "logic/Nivel.h"
#include "logic/Temporizador.h"

#include <cstddef>
#include <vector>

class Checkpoint;
class Disco;
class Jugador;
class NodoCentralEnergia;
class Obstaculo;

class NivelRutaTransmision final : public Nivel {
public:
    NivelRutaTransmision() = default;
    NivelRutaTransmision(Jugador& jugador, Disco& disco);

    std::string nombre() const override;
    void actualizar() override;
    void actualizar(std::chrono::milliseconds intervalo) override;

    void configurarActores(Jugador& jugador, Disco& disco) noexcept;
    void agregarCheckpoint(Checkpoint& checkpoint);
    void agregarObstaculo(Obstaculo& obstaculo);
    void establecerMetaFinal(NodoCentralEnergia& meta) noexcept;

    const std::vector<Checkpoint*>& obtenerCheckpoints() const noexcept;
    const std::vector<Obstaculo*>& obtenerObstaculos() const noexcept;
    const Checkpoint* checkpointActual() const noexcept;
    const NodoCentralEnergia* metaFinal() const noexcept;
    bool estaFinalizado() const noexcept;
    bool verificarLlegada(const Checkpoint& checkpoint, float tolerancia = 0.5F) const noexcept;
    bool verificarColisionConObstaculos() const noexcept;
    void reiniciarDesdeUltimoCheckpoint();

private:
    Checkpoint* objetivoActual() const noexcept;
    void iniciarTiempoObjetivo();
    void alcanzarObjetivo(Checkpoint& checkpoint);

    std::vector<Checkpoint*> checkpoints;
    std::vector<Obstaculo*> obstaculos;
    Jugador* jugador_{nullptr};
    Disco* disco_{nullptr};
    NodoCentralEnergia* metaFinal_{nullptr};
    Checkpoint* checkpointActual_{nullptr};
    std::size_t indiceObjetivo_{0};
    Temporizador temporizadorCheckpoint_;
    float toleranciaCheckpoint_{0.5F};
};
