#include "gui/MainWindow.h"

#include "logic/BarreraEstatica.h"
#include "logic/BarreraTemporizada.h"
#include "logic/NivelDefensaNucleo.h"
#include "logic/NivelRutaTransmision.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygonF>
#include <QString>
#include <QTimer>

#include <chrono>
#include <limits>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Ultimate en TRON");
    resize(960, 540);
    setMinimumSize(800, 450);
    setMouseTracking(true);

    reloj_.start();
    temporizador_ = new QTimer(this);
    temporizador_->setInterval(16);
    connect(temporizador_, &QTimer::timeout, this, [this]() {
        const auto intervalo = std::chrono::milliseconds{reloj_.restart()};
        if (pantalla_ == Pantalla::Jugando) {
            juego_.actualizar(intervalo);
        }
        update();
    });
    temporizador_->start();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_1:
        iniciarNivelRutaTransmision();
        break;
    case Qt::Key_2:
        iniciarNivelDefensaNucleo();
        break;
    case Qt::Key_Escape:
    case Qt::Key_M:
        volverAlMenu();
        break;
    case Qt::Key_R:
        reiniciarNivelActual();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QMainWindow::mousePressEvent(event);
        return;
    }

    if (pantalla_ == Pantalla::Menu) {
        const QPointF punto = event->position();
        if (botonNivel1().contains(punto)) {
            iniciarNivelRutaTransmision();
        } else if (botonNivel2().contains(punto)) {
            iniciarNivelDefensaNucleo();
        } else if (botonSalir().contains(punto)) {
            close();
        }
        QMainWindow::mousePressEvent(event);
        return;
    }

    if (!juego_.nivelActivoFinalizado()) {
        const Posicion objetivo = convertirALogica(event->position());
        if (dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual()) != nullptr) {
            defenderEn(objetivo);
        } else {
            lanzarDiscoHacia(objetivo);
        }
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

    if (pantalla_ == Pantalla::Menu) {
        dibujarMenu(painter);
        return;
    }

    dibujarNivelRutaTransmision(painter);
    dibujarNivelDefensaNucleo(painter);
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

QRectF MainWindow::botonNivel1() const {
    return QRectF{width() / 2.0 - 140.0, height() / 2.0 - 34.0, 280.0, 42.0};
}

QRectF MainWindow::botonNivel2() const {
    return QRectF{width() / 2.0 - 140.0, height() / 2.0 + 22.0, 280.0, 42.0};
}

QRectF MainWindow::botonSalir() const {
    return QRectF{width() / 2.0 - 140.0, height() / 2.0 + 78.0, 280.0, 42.0};
}

void MainWindow::iniciarNivelRutaTransmision() {
    juego_.cambiarANivelRutaTransmision();
    pantalla_ = Pantalla::Jugando;
    tieneObjetivo_ = false;
    reloj_.restart();
    update();
}

void MainWindow::iniciarNivelDefensaNucleo() {
    juego_.cambiarANivelDefensaNucleo();
    pantalla_ = Pantalla::Jugando;
    tieneObjetivo_ = false;
    reloj_.restart();
    update();
}

void MainWindow::volverAlMenu() {
    pantalla_ = Pantalla::Menu;
    tieneObjetivo_ = false;
    update();
}

void MainWindow::reiniciarNivelActual() {
    if (pantalla_ != Pantalla::Jugando) {
        return;
    }

    juego_.reiniciarNivelActivo();
    tieneObjetivo_ = false;
    reloj_.restart();
    update();
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

void MainWindow::defenderEn(const Posicion& objetivo) {
    auto* nivel = dynamic_cast<NivelDefensaNucleo*>(juego_.nivelActual());
    if (nivel == nullptr) {
        return;
    }

    const auto& discos = nivel->obtenerDiscosEnemigos();
    std::size_t indiceCercano = discos.size();
    float distanciaCercana = std::numeric_limits<float>::max();

    for (std::size_t i = 0; i < discos.size(); ++i) {
        const DiscoEnemigo* disco = discos[i];
        if (disco == nullptr || disco->estaDestruido()) {
            continue;
        }

        const float distancia = disco->posicion().distanciaA(objetivo);
        if (distancia < distanciaCercana) {
            distanciaCercana = distancia;
            indiceCercano = i;
        }
    }

    if (indiceCercano < discos.size() && distanciaCercana <= 0.75F) {
        nivel->destruirDiscoEnemigo(indiceCercano);
    }
}

void MainWindow::dibujarMenu(QPainter& painter) const {
    painter.setPen(QColor{220, 245, 255});
    painter.setFont(QFont{"Arial", 26, QFont::Bold});
    painter.drawText(QRectF{0.0, 80.0, static_cast<double>(width()), 50.0}, Qt::AlignCenter, "Ultimate en TRON");

    painter.setFont(QFont{"Arial", 12});
    painter.drawText(
        QRectF{0.0, 132.0, static_cast<double>(width()), 30.0},
        Qt::AlignCenter,
        "Selecciona un nivel para jugar");

    const QList<QPair<QRectF, QString>> botones{
        {botonNivel1(), "Jugar Nivel 1 - Ruta de Transmision"},
        {botonNivel2(), "Jugar Nivel 2 - Defensa del Nucleo"},
        {botonSalir(), "Salir"}};

    for (const auto& boton : botones) {
        painter.setPen(QPen(QColor{80, 220, 255}, 2));
        painter.setBrush(QColor{18, 35, 58});
        painter.drawRoundedRect(boton.first, 8.0, 8.0);

        painter.setPen(QColor{235, 250, 255});
        painter.setFont(QFont{"Arial", 12, QFont::Bold});
        painter.drawText(boton.first, Qt::AlignCenter, boton.second);
    }

    painter.setPen(QColor{160, 190, 210});
    painter.setFont(QFont{"Arial", 10});
    painter.drawText(
        QRectF{0.0, height() - 42.0, static_cast<double>(width()), 24.0},
        Qt::AlignCenter,
        "Atajos: 1 Nivel 1 | 2 Nivel 2 | Esc/M Menu | R Reiniciar");
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

void MainWindow::dibujarNivelDefensaNucleo(QPainter& painter) {
    const auto* nivel = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual());
    if (nivel == nullptr) {
        return;
    }

    painter.setPen(Qt::NoPen);

    const QPointF jugador = convertirAPantalla(juego_.jugador().posicion());
    painter.setBrush(QColor{40, 255, 210});
    painter.drawEllipse(jugador, 16.0, 16.0);

    for (const DiscoEnemigo* disco : nivel->obtenerDiscosEnemigos()) {
        if (disco == nullptr) {
            continue;
        }

        const QPointF centro = convertirAPantalla(disco->posicion());
        painter.setBrush(disco->estaDestruido()
            ? QColor{70, 80, 90}
            : QColor{255, 80, 90});
        painter.drawEllipse(centro, 10.0, 10.0);
    }
}

void MainWindow::dibujarEstado(QPainter& painter) const {
    painter.setPen(QColor{220, 240, 255});
    painter.setFont(QFont{"Arial", 12});
    painter.drawText(20, 30, "Ultimate en TRON");

    if (const auto* nivelDefensa = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual())) {
        painter.drawText(20, 52, "Nivel: Defensa del Nucleo");
        painter.drawText(
            20,
            74,
            QString{"Tiempo restante: %1 s"}.arg(nivelDefensa->tiempoRestante().count() / 1000.0, 0, 'f', 1));
        int discosActivos = 0;
        for (const DiscoEnemigo* disco : nivelDefensa->obtenerDiscosEnemigos()) {
            if (disco != nullptr && !disco->estaDestruido()) {
                ++discosActivos;
            }
        }
        painter.drawText(20, 96, QString{"Discos enemigos activos: %1"}.arg(discosActivos));
    } else {
        const auto* nivelRuta = dynamic_cast<const NivelRutaTransmision*>(juego_.nivelActual());
        painter.drawText(20, 52, "Nivel: Ruta de Transmision");
        if (nivelRuta != nullptr) {
            const Checkpoint* objetivo = nivelRuta->objetivoActualCheckpoint();
            const QString checkpoint = objetivo != nullptr
                ? QString::fromStdString(objetivo->id())
                : QString{"sin objetivo"};
            painter.drawText(20, 74, QString{"Checkpoint objetivo: %1"}.arg(checkpoint));
            painter.drawText(
                20,
                96,
                QString{"Tiempo restante: %1 s"}.arg(
                    nivelRuta->tiempoRestanteCheckpoint().count() / 1000.0,
                    0,
                    'f',
                    1));
        }
    }

    painter.drawText(20, 118, "Controles: clic accion | R reiniciar | Esc/M menu");

    if (!juego_.nivelActivoFinalizado()) {
        painter.drawText(20, 140, "Estado: en progreso");
    } else if (juego_.nivelActivoVictoria()) {
        painter.drawText(20, 140, "Estado: victoria");
    } else if (juego_.nivelActivoDerrota()) {
        painter.drawText(20, 140, "Estado: derrota");
    } else {
        painter.drawText(20, 140, "Estado: finalizado");
    }
}
