#pragma once

#include "logic/Juego.h"

#include <QElapsedTimer>
#include <QMainWindow>
#include <QPointF>

class QPaintEvent;
class QPainter;
class QTimer;

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPointF convertirAPantalla(const Posicion& posicion) const;
    void dibujarNivelRutaTransmision(QPainter& painter);
    void dibujarEstado(QPainter& painter) const;

    Juego juego_;
    QTimer* temporizador_{nullptr};
    QElapsedTimer reloj_;
};
