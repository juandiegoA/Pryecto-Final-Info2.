#pragma once

#include "logic/Juego.h"

#include <QElapsedTimer>
#include <QMainWindow>
#include <QPointF>

class QPaintEvent;
class QPainter;
class QMouseEvent;
class QKeyEvent;
class QTimer;

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPointF convertirAPantalla(const Posicion& posicion) const;
    Posicion convertirALogica(const QPointF& punto) const;
    void lanzarDiscoHacia(const Posicion& destino);
    void defenderEn(const Posicion& objetivo);
    void dibujarNivelRutaTransmision(QPainter& painter);
    void dibujarNivelDefensaNucleo(QPainter& painter);
    void dibujarEstado(QPainter& painter) const;

    Juego juego_;
    QTimer* temporizador_{nullptr};
    QElapsedTimer reloj_;
    Posicion ultimoObjetivo_;
    bool tieneObjetivo_{false};
};
