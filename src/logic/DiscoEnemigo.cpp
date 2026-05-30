#include "logic/DiscoEnemigo.h"

#include "logic/Jugador.h"

DiscoEnemigo::DiscoEnemigo(Posicion posicion) : Disco(posicion) {}

void DiscoEnemigo::avanzarHacia(
    const Jugador& jugador,
    float segundos,
    float velocidad) noexcept {
    if (destruido_ || segundos <= 0.0F || velocidad <= 0.0F) {
        return;
    }

    const Posicion direccion{
        jugador.posicion().x() - posicion_.x(),
        jugador.posicion().y() - posicion_.y()};
    lanzarDesde(posicion_, direccion, velocidad);
    actualizar(segundos);
}

void DiscoEnemigo::destruir() noexcept {
    destruido_ = true;
    activo_ = false;
    enMovimiento_ = false;
    velocidad_ = 0.0F;
}

bool DiscoEnemigo::estaDestruido() const noexcept {
    return destruido_;
}

bool DiscoEnemigo::impactaAl(const Jugador& jugador, float tolerancia) const noexcept {
    return !destruido_ && colisionaCon(jugador.posicion(), tolerancia);
}
