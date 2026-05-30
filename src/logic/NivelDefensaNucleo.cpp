#include "logic/NivelDefensaNucleo.h"

#include "logic/Jugador.h"

#include <algorithm>
#include <cmath>
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
    generarDiscosSiHaceFalta(intervalo);

    if (temporizadorNivel_.duracion().count() > 0
        && temporizadorNivel_.estaAgotado()) {
        declararVictoria();
        return;
    }

    const float segundos = static_cast<float>(intervalo.count()) / 1000.0F;
    moverDiscos(segundos);
    actualizarProyectilDefensor(segundos);
    verificarIntercepcionDefensiva();

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
    tiempoDesdeUltimoDisparo_ = intervaloDisparo_;
    siguienteCarril_ = 0;
    proyectilDefensorActivo_ = false;
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

void NivelDefensaNucleo::dispararDefensa(const Posicion& objetivo) noexcept {
    if (!enCurso_ || estaFinalizado() || jugador_ == nullptr) {
        return;
    }

    const Posicion origen = jugador_->posicion();
    const Posicion direccion{
        objetivo.x() - origen.x(),
        objetivo.y() - origen.y()};
    const float magnitud = std::hypot(direccion.x(), direccion.y());
    if (magnitud <= 0.01F) {
        return;
    }

    posicionProyectilDefensor_ = origen;
    objetivoProyectilDefensor_ = objetivo;
    direccionProyectilDefensor_.establecer(
        direccion.x() / magnitud,
        direccion.y() / magnitud);
    proyectilDefensorActivo_ = true;
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

const Posicion& NivelDefensaNucleo::posicionProyectilDefensor() const noexcept {
    return posicionProyectilDefensor_;
}

bool NivelDefensaNucleo::proyectilDefensorActivo() const noexcept {
    return proyectilDefensorActivo_;
}

std::size_t NivelDefensaNucleo::discosActivos() const noexcept {
    if (estaFinalizado()) {
        return 0;
    }

    std::size_t activos = 0;
    for (const DiscoEnemigo* disco : discosEnemigos) {
        if (disco != nullptr && !disco->estaDestruido()) {
            ++activos;
        }
    }

    return activos;
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

void NivelDefensaNucleo::actualizarProyectilDefensor(float segundos) noexcept {
    if (!proyectilDefensorActivo_ || segundos <= 0.0F) {
        return;
    }

    posicionProyectilDefensor_.establecer(
        posicionProyectilDefensor_.x() + direccionProyectilDefensor_.x() * velocidadProyectilDefensor_ * segundos,
        posicionProyectilDefensor_.y() + direccionProyectilDefensor_.y() * velocidadProyectilDefensor_ * segundos);

    const Posicion restante{
        objetivoProyectilDefensor_.x() - posicionProyectilDefensor_.x(),
        objetivoProyectilDefensor_.y() - posicionProyectilDefensor_.y()};
    const float avancePendiente =
        restante.x() * direccionProyectilDefensor_.x()
        + restante.y() * direccionProyectilDefensor_.y();
    if (avancePendiente <= 0.0F) {
        proyectilDefensorActivo_ = false;
    }
}

void NivelDefensaNucleo::verificarIntercepcionDefensiva() noexcept {
    if (!proyectilDefensorActivo_) {
        return;
    }

    for (DiscoEnemigo* disco : discosEnemigos) {
        if (disco != nullptr
            && !disco->estaDestruido()
            && disco->posicion().distanciaA(posicionProyectilDefensor_) <= toleranciaIntercepcion_) {
            disco->destruir();
            proyectilDefensorActivo_ = false;
            return;
        }
    }
}

void NivelDefensaNucleo::generarDiscosSiHaceFalta(std::chrono::milliseconds intervalo) {
    if (!enCurso_ || intervalo.count() <= 0 || discosActivos() >= maxDiscosActivos_) {
        return;
    }

    tiempoDesdeUltimoDisparo_ += intervalo;
    if (tiempoDesdeUltimoDisparo_ < intervaloDisparo_) {
        return;
    }

    const Posicion carriles[] = {
        Posicion{-3.5F, 9.5F},
        Posicion{0.0F, 10.5F},
        Posicion{3.5F, 9.5F},
        Posicion{-1.8F, 8.5F},
        Posicion{1.8F, 8.5F}
    };
    const Posicion origen = carriles[siguienteCarril_ % std::size(carriles)];
    ++siguienteCarril_;
    generarDiscoEnemigo(origen);
    tiempoDesdeUltimoDisparo_ = std::chrono::milliseconds{0};
}

void NivelDefensaNucleo::desactivarDiscosEnemigos() noexcept {
    for (DiscoEnemigo* disco : discosEnemigos) {
        if (disco != nullptr) {
            disco->destruir();
        }
    }
    proyectilDefensorActivo_ = false;
}

void NivelDefensaNucleo::declararVictoria() noexcept {
    victoria_ = true;
    derrota_ = false;
    enCurso_ = false;
    temporizadorNivel_.detener();
    desactivarDiscosEnemigos();
}

void NivelDefensaNucleo::declararDerrota() noexcept {
    derrota_ = true;
    victoria_ = false;
    enCurso_ = false;
    temporizadorNivel_.detener();
    desactivarDiscosEnemigos();
}
