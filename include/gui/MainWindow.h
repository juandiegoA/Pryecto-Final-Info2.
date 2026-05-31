#pragma once

#include "logic/Juego.h"

#include <QElapsedTimer>
#include <QMainWindow>
#include <QPointF>
#include <QRectF>
#include <QString>

#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <string>

class QPaintEvent;
class QPainter;
class QMouseEvent;
class QKeyEvent;
class QSoundEffect;
class QTimer;

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    enum class Pantalla {
        Menu,
        Jugando
    };

    QPointF convertirAPantalla(const Posicion& posicion) const;
    Posicion convertirALogica(const QPointF& punto) const;
    QPointF convertirDefensaAPantalla(const Posicion& posicion) const;
    Posicion convertirDefensaALogica(const QPointF& punto) const;
    double escalaDefensa(const Posicion& posicion) const;
    QRectF botonNivel1() const;
    QRectF botonNivel2() const;
    QRectF botonSalir() const;
    QRectF botonDificultadFacil() const;
    QRectF botonDificultadMedio() const;
    QRectF botonDificultadDificil() const;
    void iniciarNivelRutaTransmision();
    void iniciarNivelDefensaNucleo();
    void volverAlMenu();
    void reiniciarNivelActual();
    void manejarClickMenu(const QPointF& punto);
    void manejarClickJuego(const QPointF& punto);
    void actualizarEfectosVisuales(std::chrono::milliseconds intervalo);
    void reiniciarEfectosVisuales();
    bool ejecutarAccionSegura(const std::function<void()>& accion);
    void registrarError(const std::exception& error);
    void inicializarSonido();
    void reproducirSonidoCheckpoint();
    void reproducirSonidoImpacto();
    void reproducirSonidoVictoria();
    void reproducirSonidoDerrota();
    void reproducirSonido(QSoundEffect* sonido) const;
    void actualizarMusicaFondo();
    void lanzarDiscoHacia(const Posicion& destino);
    void defenderEn(const Posicion& objetivo);
    void dibujarFondo(QPainter& painter) const;
    void dibujarMenu(QPainter& painter) const;
    void dibujarNivelRutaTransmision(QPainter& painter);
    void dibujarNivelDefensaNucleo(QPainter& painter);
    void dibujarEstado(QPainter& painter) const;
    void dibujarOverlayFinal(QPainter& painter) const;

    Juego juego_;
    QTimer* temporizador_{nullptr};
    QSoundEffect* musicaMenu_{nullptr};
    QSoundEffect* sonidoCheckpoint_{nullptr};
    QSoundEffect* sonidoImpacto_{nullptr};
    QSoundEffect* sonidoVictoria_{nullptr};
    QSoundEffect* sonidoDerrota_{nullptr};
    QElapsedTimer reloj_;
    Posicion ultimoObjetivo_;
    Posicion posicionPulsoCheckpoint_;
    Posicion posicionPulsoImpacto_;
    QPointF posicionCursor_;
    QString mensajeError_;
    std::chrono::milliseconds pulsoCheckpoint_{0};
    std::chrono::milliseconds pulsoImpacto_{0};
    std::chrono::milliseconds pulsoFinal_{0};
    std::string ultimoCheckpointActivado_;
    std::size_t ultimasAmenazasActivas_{0};
    bool tieneObjetivo_{false};
    bool cursorSobreVentana_{false};
    bool finalRegistrado_{false};
    Pantalla pantalla_{Pantalla::Menu};
};
