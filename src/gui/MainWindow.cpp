#include "gui/MainWindow.h"

#include "logic/BarreraEstatica.h"
#include "logic/BarreraTemporizada.h"
#include "logic/NivelRutaTransmision.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygonF>
#include <QTimer>

#include <chrono>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Ultimate en TRON");
    resize(960, 540);
    setMinimumSize(800, 450);
    setMouseTracking(true);

    juego_.crearNivelRutaTransmision();

    reloj_.start();
    temporizador_ = new QTimer(this);
    temporizador_->setInterval(16);
    connect(temporizador_, &QTimer::timeout, this, [this]() {
        const auto intervalo = std::chrono::milliseconds{reloj_.restart()};
        juego_.actualizar(intervalo);
        update();
    });
    temporizador_->start();
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && !juego_.nivelActivoFinalizado()) {
        lanzarDiscoHacia(convertirALogica(event->position()));
        update();
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QMainWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor{12, 16, 28});

    painter.setPen(QPen(QColor{30, 180, 220}, 1));
    for (int x = 40; x < width(); x += 40) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 40; y < height(); y += 40) {
        painter.drawLine(0, y, width(), y);
    }

    dibujarNivelRutaTransmision(painter);
    dibujarEstado(painter);
}

QPointF MainWindow::convertirAPantalla(const Posicion& posicion) const {
    constexpr double escala = 55.0;
    const double origenX = 80.0;
    const double origenY = height() / 2.0;
    return QPointF{
        origenX + posicion.x() * escala,
        origenY - posicion.y() * escala};
}

Posicion MainWindow::convertirALogica(const QPointF& punto) const {
    constexpr double escala = 55.0;
    const double origenX = 80.0;
    const double origenY = height() / 2.0;
    return Posicion{
        static_cast<float>((punto.x() - origenX) / escala),
        static_cast<float>((origenY - punto.y()) / escala)};
}

void MainWindow::lanzarDiscoHacia(const Posicion& destino) {
    const Posicion origen = juego_.jugador().posicion();
    const Posicion direccion{
        destino.x() - origen.x(),
        destino.y() - origen.y()};

    if (direccion.distanciaA(Posicion{}) <= 0.01F) {
        return;
    }

    ultimoObjetivo_ = destino;
    tieneObjetivo_ = true;
    juego_.discoJugador().lanzarDesde(origen, direccion, 3.5F);
}

void MainWindow::dibujarNivelRutaTransmision(QPainter& painter) {
    const auto* nivel = dynamic_cast<const NivelRutaTransmision*>(juego_.nivelActual());
    if (nivel == nullptr) {
        return;
    }

    painter.setPen(Qt::NoPen);

    for (const Checkpoint* checkpoint : nivel->obtenerCheckpoints()) {
        if (checkpoint == nullptr) {
            continue;
        }

        const QPointF centro = convertirAPantalla(checkpoint->posicion());
        painter.setBrush(checkpoint->estaActivado()
            ? QColor{70, 230, 140}
            : QColor{240, 200, 70});
        painter.drawEllipse(centro, 12.0, 12.0);
    }

    if (const NodoCentralEnergia* meta = nivel->metaFinal()) {
        const QPointF centro = convertirAPantalla(meta->posicion());
        QPolygonF rombo;
        rombo << QPointF{centro.x(), centro.y() - 18.0}
              << QPointF{centro.x() + 18.0, centro.y()}
              << QPointF{centro.x(), centro.y() + 18.0}
              << QPointF{centro.x() - 18.0, centro.y()};
        painter.setBrush(QColor{180, 90, 255});
        painter.drawPolygon(rombo);
    }

    for (const Obstaculo* obstaculo : nivel->obtenerObstaculos()) {
        if (const auto* barrera = dynamic_cast<const BarreraEstatica*>(obstaculo)) {
            const QPointF centro = convertirAPantalla(barrera->posicion());
            painter.setBrush(QColor{230, 70, 70});
            painter.drawRect(QRectF{centro.x() - 14.0, centro.y() - 28.0, 28.0, 56.0});
        } else if (const auto* barrera = dynamic_cast<const BarreraTemporizada*>(obstaculo)) {
            const QPointF centro = convertirAPantalla(barrera->posicion());
            painter.setBrush(barrera->estaActivo()
                ? QColor{255, 120, 70}
                : QColor{70, 90, 120});
            painter.drawRect(QRectF{centro.x() - 14.0, centro.y() - 28.0, 28.0, 56.0});
        }
    }

    for (const Dron* dron : nivel->obtenerDrones()) {
        if (dron == nullptr) {
            continue;
        }

        const QPointF centro = convertirAPantalla(dron->posicion());
        painter.setBrush(QColor{80, 180, 255});
        painter.drawEllipse(centro, 10.0, 10.0);
    }

    const QPointF jugador = convertirAPantalla(juego_.jugador().posicion());
    if (tieneObjetivo_) {
        painter.setPen(QPen(QColor{120, 240, 255}, 2, Qt::DashLine));
        painter.drawLine(jugador, convertirAPantalla(ultimoObjetivo_));
        painter.setPen(Qt::NoPen);
    }

    painter.setBrush(QColor{40, 255, 210});
    painter.drawEllipse(jugador, 14.0, 14.0);

    const QPointF disco = convertirAPantalla(juego_.discoJugador().posicion());
    painter.setBrush(juego_.discoJugador().estaActivo()
        ? QColor{255, 255, 255}
        : QColor{130, 140, 150});
    painter.drawEllipse(disco, 7.0, 7.0);
}

void MainWindow::dibujarEstado(QPainter& painter) const {
    painter.setPen(QColor{220, 240, 255});
    painter.setFont(QFont{"Arial", 12});
    painter.drawText(20, 30, "Ultimate en TRON - prueba visual de logica");
    painter.drawText(20, 52, "Nivel: Ruta de Transmision");
    painter.drawText(20, 74, "Input: clic izquierdo para lanzar el disco");

    if (!juego_.nivelActivoFinalizado()) {
        painter.drawText(20, 96, "Estado: en progreso");
    } else if (juego_.nivelActivoVictoria()) {
        painter.drawText(20, 96, "Estado: victoria");
    } else if (juego_.nivelActivoDerrota()) {
        painter.drawText(20, 96, "Estado: derrota");
    } else {
        painter.drawText(20, 96, "Estado: finalizado");
    }
}
