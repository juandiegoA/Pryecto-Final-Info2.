#include "logic/Dron.h"

#include "logic/Disco.h"
#include "logic/Jugador.h"

#include <algorithm>
#include <utility>

Dron::Dron(Posicion posicion) : posicion_(posicion), objetivo_(posicion) {}

Dron::Dron(Posicion posicion, float velocidad)
    : posicion_(posicion), objetivo_(posicion) {
    establecerVelocidad(velocidad);
}

const Posicion& Dron::posicion() const noexcept {
    return posicion_;
}

float Dron::velocidad() const noexcept {
    return velocidad_;
}

bool Dron::estaActivo() const noexcept {
    return activo_;
}

const std::vector<Posicion>& Dron::rutaVigilancia() const noexcept {
    return rutaVigilancia_;
}

void Dron::establecerVelocidad(float velocidad) noexcept {
    velocidad_ = std::max(velocidad, 0.0F);
}

void Dron::activar() noexcept {
    activo_ = true;
}

void Dron::desactivar() noexcept {
    activo_ = false;
}

void Dron::vigilarZona(const Posicion& objetivo) noexcept {
    objetivo_ = objetivo;
    tieneObjetivo_ = true;
    siguiendoRuta_ = false;
}

void Dron::definirRutaVigilancia(std::vector<Posicion> ruta) {
    rutaVigilancia_ = std::move(ruta);
    indiceRuta_ = 0;
    siguiendoRuta_ = !rutaVigilancia_.empty();
    tieneObjetivo_ = siguiendoRuta_;

    if (siguiendoRuta_) {
        objetivo_ = rutaVigilancia_[indiceRuta_];
    }
}

void Dron::intentarInterceptar(const Disco& disco, const Jugador& jugador) noexcept {
    if (disco.estaActivo() && disco.estaEnMovimiento()) {
        const float horizonte = 0.75F;
        vigilarZona(Posicion{
            disco.posicion().x() + disco.direccion().x() * disco.velocidad() * horizonte,
            disco.posicion().y() + disco.direccion().y() * disco.velocidad() * horizonte});
        return;
    }

    vigilarZona(jugador.posicion());
}

void Dron::actualizar(float segundos) noexcept {
    if (!activo_ || segundos <= 0.0F || !tieneObjetivo_) {
        return;
    }

    moverHaciaObjetivo(segundos);

    if (siguiendoRuta_ && !rutaVigilancia_.empty()
        && posicion_.distanciaA(objetivo_) <= 0.05F) {
        indiceRuta_ = (indiceRuta_ + 1U) % rutaVigilancia_.size();
        objetivo_ = rutaVigilancia_[indiceRuta_];
    }
}

bool Dron::bloqueaDisco(const Disco& disco, float tolerancia) const noexcept {
    return activo_ && disco.cruzaPor(posicion_, tolerancia);
}

void Dron::moverHaciaObjetivo(float segundos) noexcept {
    const float distancia = posicion_.distanciaA(objetivo_);
    if (distancia <= 0.0F) {
        return;
    }

    const float avance = std::min(velocidad_ * segundos, distancia);
    const float factor = avance / distancia;
    posicion_.establecer(
        posicion_.x() + (objetivo_.x() - posicion_.x()) * factor,
        posicion_.y() + (objetivo_.y() - posicion_.y()) * factor);
}
