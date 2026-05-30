#include "logic/NivelDefensaNucleo.h"

#include "logic/Jugador.h"

#include <algorithm>
#include <utility>

NivelDefensaNucleo::NivelDefensaNucleo(
    Jugador& jugador,
    std::chrono::milliseconds tiempoObjetivo)
    : jugador_(&jugador), temporizadorNivel_(tiempoObjetivo) {}

std::string NivelDefensaNucleo::nombre() const {
    return "Defensa del Nucleo";
}

void NivelDefensaNucleo::actualizar() {
    actualizar(std::chrono::milliseconds{0});
}

void NivelDefensaNucleo::actualizar(std::chrono::milliseconds intervalo) {
    if (estaFinalizado() || jugador_ == nullptr) {
        return;
    }

    if (!enCurso_) {
        iniciar();
    }

    temporizadorNivel_.actualizar(intervalo);

    if (temporizadorNivel_.duracion().count() > 0
        && temporizadorNivel_.estaAgotado()) {
        declararVictoria();
        return;
    }

    const float segundos = static_cast<float>(intervalo.count()) / 1000.0F;
    moverDiscos(segundos);

    if (verificarImpactoJugador()) {
        declararDerrota();
        return;
    }

}

void NivelDefensaNucleo::configurarJugador(Jugador& jugador) noexcept {
    jugador_ = &jugador;
}

void NivelDefensaNucleo::configurarTiempoObjetivo(
    std::chrono::milliseconds tiempoObjetivo) {
    temporizadorNivel_ = Temporizador{std::max(tiempoObjetivo, std::chrono::milliseconds{0})};
    enCurso_ = false;
    victoria_ = false;
    derrota_ = false;
}

void NivelDefensaNucleo::iniciar() {
    temporizadorNivel_.iniciar();
    enCurso_ = true;
    victoria_ = false;
    derrota_ = false;
}

void NivelDefensaNucleo::agregarDiscoEnemigo(DiscoEnemigo& disco) {
    discosEnemigos.push_back(&disco);
}

DiscoEnemigo& NivelDefensaNucleo::generarDiscoEnemigo(Posicion posicion) {
    auto disco = std::make_unique<DiscoEnemigo>(posicion);
    DiscoEnemigo& referencia = *disco;
    discosGenerados_.push_back(std::move(disco));
    discosEnemigos.push_back(&referencia);
    return referencia;
}

bool NivelDefensaNucleo::destruirDiscoEnemigo(std::size_t indice) noexcept {
    if (indice >= discosEnemigos.size() || discosEnemigos[indice] == nullptr) {
        return false;
    }

    discosEnemigos[indice]->destruir();
    return true;
}

bool NivelDefensaNucleo::verificarImpactoJugador() const noexcept {
    if (jugador_ == nullptr) {
        return false;
    }

    for (const DiscoEnemigo* disco : discosEnemigos) {
        if (disco != nullptr && disco->impactaAl(*jugador_, toleranciaImpacto_)) {
            return true;
        }
    }

    return false;
}

const std::vector<DiscoEnemigo*>& NivelDefensaNucleo::obtenerDiscosEnemigos() const noexcept {
    return discosEnemigos;
}

std::chrono::milliseconds NivelDefensaNucleo::tiempoTranscurrido() const noexcept {
    return temporizadorNivel_.tiempoTranscurrido();
}

std::chrono::milliseconds NivelDefensaNucleo::tiempoRestante() const noexcept {
    return temporizadorNivel_.tiempoRestante();
}

bool NivelDefensaNucleo::estaEnCurso() const noexcept {
    return enCurso_;
}

bool NivelDefensaNucleo::estaFinalizado() const {
    return victoria_ || derrota_;
}

bool NivelDefensaNucleo::victoria() const {
    return victoria_;
}

bool NivelDefensaNucleo::derrota() const {
    return derrota_;
}

void NivelDefensaNucleo::moverDiscos(float segundos) noexcept {
    for (DiscoEnemigo* disco : discosEnemigos) {
        if (disco != nullptr) {
            disco->avanzarHacia(*jugador_, segundos, velocidadDiscoEnemigo_);
        }
    }
}

void NivelDefensaNucleo::declararVictoria() noexcept {
    victoria_ = true;
    derrota_ = false;
    enCurso_ = false;
    temporizadorNivel_.detener();
}

void NivelDefensaNucleo::declararDerrota() noexcept {
    derrota_ = true;
    victoria_ = false;
    enCurso_ = false;
    temporizadorNivel_.detener();
}
