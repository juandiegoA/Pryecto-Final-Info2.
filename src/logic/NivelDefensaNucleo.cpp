#include "logic/NivelDefensaNucleo.h"

#include "logic/Jugador.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

NivelDefensaNucleo::NivelDefensaNucleo(
    Jugador& jugador,
    std::chrono::milliseconds tiempoObjetivo)
    : jugador_(&jugador), temporizadorNivel_(tiempoObjetivo) {
    if (tiempoObjetivo.count() <= 0) {
        throw std::invalid_argument{"NivelDefensaNucleo requiere un tiempo objetivo positivo"};
    }
    configurarDificultad(dificultad_);
}

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
    tiempoDesdeUltimoDisparoDefensor_ += intervalo;
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

void NivelDefensaNucleo::configurarDificultad(DificultadDefensa dificultad) {
    dificultad_ = dificultad;
    const ConfiguracionDificultad configuracion = crearConfiguracion(dificultad);

    carrilesAparicion_ = configuracion.carriles;
    intervaloDisparo_ = configuracion.intervaloDisparoEnemigo;
    intervaloProyectilDefensor_ = configuracion.intervaloProyectilDefensor;
    velocidadDiscoEnemigo_ = configuracion.velocidadDiscoEnemigo;
    velocidadProyectilDefensor_ = configuracion.velocidadProyectilDefensor;
    toleranciaIntercepcion_ = configuracion.toleranciaIntercepcion;
    maxDiscosActivos_ = configuracion.maxDiscosActivos;
}

void NivelDefensaNucleo::configurarJugador(Jugador& jugador) noexcept {
    jugador_ = &jugador;
}

void NivelDefensaNucleo::configurarTiempoObjetivo(
    std::chrono::milliseconds tiempoObjetivo) {
    if (tiempoObjetivo.count() <= 0) {
        throw std::invalid_argument{"El tiempo objetivo de NivelDefensaNucleo debe ser positivo"};
    }

    temporizadorNivel_ = Temporizador{std::max(tiempoObjetivo, std::chrono::milliseconds{0})};
    enCurso_ = false;
    victoria_ = false;
    derrota_ = false;
}

void NivelDefensaNucleo::iniciar() {
    if (jugador_ == nullptr) {
        throw std::logic_error{"No se puede iniciar NivelDefensaNucleo sin jugador configurado"};
    }
    if (temporizadorNivel_.duracion().count() <= 0) {
        throw std::logic_error{"No se puede iniciar NivelDefensaNucleo sin tiempo objetivo valido"};
    }
    if (carrilesAparicion_.empty()) {
        throw std::logic_error{"No se puede iniciar NivelDefensaNucleo sin carriles de aparicion"};
    }

    temporizadorNivel_.iniciar();
    tiempoDesdeUltimoDisparo_ = intervaloDisparo_;
    tiempoDesdeUltimoDisparoDefensor_ = intervaloProyectilDefensor_;
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
    if (tiempoDesdeUltimoDisparoDefensor_ < intervaloProyectilDefensor_) {
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
    tiempoDesdeUltimoDisparoDefensor_ = std::chrono::milliseconds{0};
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

DiscoEnemigo& NivelDefensaNucleo::discoEnemigoEn(std::size_t indice) {
    if (indice >= discosEnemigos.size()) {
        throw std::out_of_range{"Indice de disco enemigo fuera de rango"};
    }
    if (discosEnemigos[indice] == nullptr) {
        throw std::logic_error{"Disco enemigo no configurado en NivelDefensaNucleo"};
    }

    return *discosEnemigos[indice];
}

const DiscoEnemigo& NivelDefensaNucleo::discoEnemigoEn(std::size_t indice) const {
    if (indice >= discosEnemigos.size()) {
        throw std::out_of_range{"Indice de disco enemigo fuera de rango"};
    }
    if (discosEnemigos[indice] == nullptr) {
        throw std::logic_error{"Disco enemigo no configurado en NivelDefensaNucleo"};
    }

    return *discosEnemigos[indice];
}

const Posicion& NivelDefensaNucleo::posicionProyectilDefensor() const noexcept {
    return posicionProyectilDefensor_;
}

bool NivelDefensaNucleo::proyectilDefensorActivo() const noexcept {
    return proyectilDefensorActivo_;
}

DificultadDefensa NivelDefensaNucleo::dificultad() const noexcept {
    return dificultad_;
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

NivelDefensaNucleo::ConfiguracionDificultad NivelDefensaNucleo::crearConfiguracion(
    DificultadDefensa dificultad) {
    switch (dificultad) {
    case DificultadDefensa::Facil:
        return ConfiguracionDificultad{
            {
                Posicion{-2.8F, 9.0F},
                Posicion{0.0F, 9.8F},
                Posicion{2.8F, 9.0F}
            },
            std::chrono::milliseconds{2600},
            std::chrono::milliseconds{180},
            1.65F,
            9.4F,
            0.62F,
            4};
    case DificultadDefensa::Dificil:
        return ConfiguracionDificultad{
            {
                Posicion{-3.8F, 10.0F},
                Posicion{-1.3F, 8.7F},
                Posicion{3.8F, 10.0F},
                Posicion{1.3F, 8.7F},
                Posicion{0.0F, 10.8F},
                Posicion{-2.4F, 9.3F},
                Posicion{2.4F, 9.3F}
            },
            std::chrono::milliseconds{1600},
            std::chrono::milliseconds{520},
            2.65F,
            7.4F,
            0.34F,
            6};
    case DificultadDefensa::Medio:
        return ConfiguracionDificultad{
            {
                Posicion{-3.5F, 9.5F},
                Posicion{0.0F, 10.5F},
                Posicion{3.5F, 9.5F},
                Posicion{-1.8F, 8.5F},
                Posicion{1.8F, 8.5F}
            },
            std::chrono::milliseconds{2200},
            std::chrono::milliseconds{300},
            2.0F,
            8.0F,
            0.45F,
            5};
    }

    throw std::invalid_argument{"Dificultad de NivelDefensaNucleo no reconocida"};
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

    if (carrilesAparicion_.empty()) {
        carrilesAparicion_ = crearConfiguracion(dificultad_).carriles;
    }

    const Posicion origen = carrilesAparicion_[siguienteCarril_ % carrilesAparicion_.size()];
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
