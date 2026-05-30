#include "logic/AgenteInteligente.h"

#include "logic/Checkpoint.h"
#include "logic/Disco.h"
#include "logic/Dron.h"
#include "logic/Jugador.h"

#include <algorithm>

AgenteInteligente::AgenteInteligente(Posicion posicion)
    : posicion_(posicion), objetivoVigilancia_(posicion) {}

const Posicion& AgenteInteligente::posicion() const noexcept {
    return posicion_;
}

const Posicion& AgenteInteligente::objetivoVigilancia() const noexcept {
    return objetivoVigilancia_;
}

const std::vector<int>& AgenteInteligente::rutasFrecuentes() const noexcept {
    return rutasFrecuentes_;
}

void AgenteInteligente::percibir(
    const Jugador& jugador,
    const Disco& disco,
    const std::vector<Checkpoint*>& checkpoints) {
    posicionJugador_ = jugador.posicion();
    posicionDisco_ = disco.posicion();
    direccionDisco_ = disco.direccion();
    velocidadDisco_ = disco.velocidad();
    discoActivo_ = disco.estaActivo();
    discoEnMovimiento_ = disco.estaEnMovimiento();

    const int rutaFrecuente = elegirRutaFrecuente(checkpoints.size());
    if (rutaFrecuente >= 0) {
        indiceRutaProbable_ = static_cast<std::size_t>(rutaFrecuente);
    } else {
        indiceRutaProbable_ = checkpoints.size();
        for (std::size_t i = 0; i < checkpoints.size(); ++i) {
            if (checkpoints[i] != nullptr && !checkpoints[i]->estaActivado()) {
                indiceRutaProbable_ = i;
                break;
            }
        }
    }

    if (indiceRutaProbable_ < checkpoints.size()
        && checkpoints[indiceRutaProbable_] != nullptr) {
        objetivoVigilancia_ = checkpoints[indiceRutaProbable_]->posicion();
        tieneObjetivo_ = true;
        return;
    }

    objetivoVigilancia_ = discoActivo_ ? posicionDisco_ : posicionJugador_;
    tieneObjetivo_ = true;
}

std::size_t AgenteInteligente::estimarIndiceRutaProbable() const noexcept {
    return indiceRutaProbable_;
}

void AgenteInteligente::instruir(Dron& dron) const noexcept {
    if (discoActivo_ && discoEnMovimiento_) {
        Jugador jugadorEstimado{posicionJugador_};
        Disco discoEstimado{posicionDisco_};
        discoEstimado.lanzarDesde(posicionDisco_, direccionDisco_, velocidadDisco_);
        dron.intentarInterceptar(discoEstimado, jugadorEstimado);
        return;
    }

    if (tieneObjetivo_) {
        dron.vigilarZona(objetivoVigilancia_);
    }
}

void AgenteInteligente::registrarRutaFrecuente(int indiceRuta) {
    if (indiceRuta >= 0) {
        rutasFrecuentes_.push_back(indiceRuta);
    }
}

int AgenteInteligente::elegirRutaFrecuente(std::size_t cantidadRutas) const noexcept {
    int mejorRuta = -1;
    int mejorConteo = 0;

    for (int ruta : rutasFrecuentes_) {
        if (ruta < 0 || static_cast<std::size_t>(ruta) >= cantidadRutas) {
            continue;
        }

        const int conteo = static_cast<int>(std::count(
            rutasFrecuentes_.begin(),
            rutasFrecuentes_.end(),
            ruta));

        if (conteo > mejorConteo) {
            mejorConteo = conteo;
            mejorRuta = ruta;
        }
    }

    return mejorRuta;
}
