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
#include <QSoundEffect>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <vector>

namespace {
constexpr int anchoInicialVentana = 960;
constexpr int altoInicialVentana = 540;
constexpr int anchoMinimoVentana = 800;
constexpr int altoMinimoVentana = 450;
constexpr int intervaloActualizacionMs = 16;
constexpr int separacionGrid = 40;

constexpr double escalaRuta = 55.0;
constexpr double origenRutaX = 80.0;
constexpr double profundidadMaximaDefensa = 10.5;
constexpr double margenInferiorDefensa = 82.0;
constexpr double horizonteDefensaY = 92.0;
constexpr double anchoCarrilDefensa = 54.0;
constexpr double variacionCarrilDefensa = 18.0;

constexpr double anchoBotonMenu = 280.0;
constexpr double altoBotonMenu = 42.0;
constexpr double anchoBotonDificultad = 84.0;
constexpr double altoBotonDificultad = 34.0;

constexpr float volumenMusicaMenu = 0.18F;
constexpr float volumenCheckpoint = 0.55F;
constexpr float volumenImpacto = 0.6F;
constexpr float volumenResultado = 0.65F;

constexpr const char* audioMenu = "qrc:/audio/menu_loop.wav";
constexpr const char* audioCheckpoint = "qrc:/audio/checkpoint.wav";
constexpr const char* audioImpacto = "qrc:/audio/destroy_enemy.wav";
constexpr const char* audioVictoria = "qrc:/audio/victory.wav";
constexpr const char* audioDerrota = "qrc:/audio/defeat.wav";

constexpr auto duracionPulsoCheckpoint = std::chrono::milliseconds{620};
constexpr auto duracionPulsoImpacto = std::chrono::milliseconds{460};
constexpr auto duracionPulsoFinal = std::chrono::milliseconds{1200};

double progresoPulso(std::chrono::milliseconds restante, std::chrono::milliseconds duracion) {
    if (duracion.count() <= 0) {
        return 1.0;
    }

    return 1.0 - std::clamp(
        static_cast<double>(restante.count()) / static_cast<double>(duracion.count()),
        0.0,
        1.0);
}

QString textoDificultad(DificultadDefensa dificultad) {
    switch (dificultad) {
    case DificultadDefensa::Facil:
        return "Facil";
    case DificultadDefensa::Dificil:
        return "Dificil";
    case DificultadDefensa::Medio:
    default:
        return "Medio";
    }
}

QSoundEffect* crearEfectoSonido(QObject* parent, const char* recurso, float volumen, int repeticiones = 1) {
    auto* sonido = new QSoundEffect(parent);
    sonido->setSource(QUrl{recurso});
    sonido->setLoopCount(repeticiones);
    sonido->setVolume(volumen);
    return sonido;
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Ultimate en TRON");
    resize(anchoInicialVentana, altoInicialVentana);
    setMinimumSize(anchoMinimoVentana, altoMinimoVentana);
    setMouseTracking(true);
    inicializarSonido();

    reloj_.start();
    temporizador_ = new QTimer(this);
    temporizador_->setInterval(intervaloActualizacionMs);
    connect(temporizador_, &QTimer::timeout, this, [this]() {
        const auto intervalo = std::chrono::milliseconds{reloj_.restart()};
        if (pantalla_ == Pantalla::Jugando) {
            ejecutarAccionSegura([this, intervalo]() {
                juego_.actualizar(intervalo);
                actualizarEfectosVisuales(intervalo);
            });
        }
        update();
    });
    temporizador_->start();
    actualizarMusicaFondo();
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

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    posicionCursor_ = event->position();
    cursorSobreVentana_ = true;
    update();
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QMainWindow::mousePressEvent(event);
        return;
    }

    if (pantalla_ == Pantalla::Menu) {
        manejarClickMenu(event->position());
        QMainWindow::mousePressEvent(event);
        return;
    }

    manejarClickJuego(event->position());

    QMainWindow::mousePressEvent(event);
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QMainWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    dibujarFondo(painter);

    if (pantalla_ == Pantalla::Menu) {
        dibujarMenu(painter);
        return;
    }

    dibujarNivelRutaTransmision(painter);
    dibujarNivelDefensaNucleo(painter);
    dibujarEstado(painter);
    dibujarOverlayFinal(painter);
}

QPointF MainWindow::convertirAPantalla(const Posicion& posicion) const {
    const double origenY = height() / 2.0;
    return QPointF{
        origenRutaX + posicion.x() * escalaRuta,
        origenY - posicion.y() * escalaRuta};
}

Posicion MainWindow::convertirALogica(const QPointF& punto) const {
    const double origenY = height() / 2.0;
    return Posicion{
        static_cast<float>((punto.x() - origenRutaX) / escalaRuta),
        static_cast<float>((origenY - punto.y()) / escalaRuta)};
}

QPointF MainWindow::convertirDefensaAPantalla(const Posicion& posicion) const {
    const double centroX = width() / 2.0;
    const double fondoY = height() - margenInferiorDefensa;
    const double t = std::clamp(static_cast<double>(posicion.y()) / profundidadMaximaDefensa, 0.0, 1.0);
    const double ancho = anchoCarrilDefensa + variacionCarrilDefensa * (1.0 - t);

    return QPointF{
        centroX + posicion.x() * ancho,
        fondoY - (fondoY - horizonteDefensaY) * t};
}

Posicion MainWindow::convertirDefensaALogica(const QPointF& punto) const {
    const double centroX = width() / 2.0;
    const double fondoY = height() - margenInferiorDefensa;
    const double t = std::clamp((fondoY - punto.y()) / (fondoY - horizonteDefensaY), 0.0, 1.0);
    const double ancho = anchoCarrilDefensa + variacionCarrilDefensa * (1.0 - t);

    return Posicion{
        static_cast<float>((punto.x() - centroX) / ancho),
        static_cast<float>(t * profundidadMaximaDefensa)};
}

double MainWindow::escalaDefensa(const Posicion& posicion) const {
    const double t = std::clamp(static_cast<double>(posicion.y()) / profundidadMaximaDefensa, 0.0, 1.0);
    return 0.58 + (1.0 - t) * 0.72;
}

QRectF MainWindow::botonNivel1() const {
    return QRectF{
        width() / 2.0 - anchoBotonMenu / 2.0,
        height() / 2.0 - 34.0,
        anchoBotonMenu,
        altoBotonMenu};
}

QRectF MainWindow::botonNivel2() const {
    return QRectF{
        width() / 2.0 - anchoBotonMenu / 2.0,
        height() / 2.0 + 22.0,
        anchoBotonMenu,
        altoBotonMenu};
}

QRectF MainWindow::botonSalir() const {
    return QRectF{
        width() / 2.0 - anchoBotonMenu / 2.0,
        height() / 2.0 + 78.0,
        anchoBotonMenu,
        altoBotonMenu};
}

QRectF MainWindow::botonDificultadFacil() const {
    return QRectF{width() / 2.0 - 140.0, height() / 2.0 + 142.0, anchoBotonDificultad, altoBotonDificultad};
}

QRectF MainWindow::botonDificultadMedio() const {
    return QRectF{width() / 2.0 - 42.0, height() / 2.0 + 142.0, anchoBotonDificultad, altoBotonDificultad};
}

QRectF MainWindow::botonDificultadDificil() const {
    return QRectF{width() / 2.0 + 56.0, height() / 2.0 + 142.0, anchoBotonDificultad, altoBotonDificultad};
}

void MainWindow::iniciarNivelRutaTransmision() {
    ejecutarAccionSegura([this]() {
        juego_.cambiarANivelRutaTransmision();
        pantalla_ = Pantalla::Jugando;
        mensajeError_.clear();
        tieneObjetivo_ = false;
        reiniciarEfectosVisuales();
        reloj_.restart();
        actualizarMusicaFondo();
        update();
    });
}

void MainWindow::iniciarNivelDefensaNucleo() {
    ejecutarAccionSegura([this]() {
        juego_.cambiarANivelDefensaNucleo();
        pantalla_ = Pantalla::Jugando;
        mensajeError_.clear();
        tieneObjetivo_ = false;
        reiniciarEfectosVisuales();
        reloj_.restart();
        actualizarMusicaFondo();
        update();
    });
}

void MainWindow::volverAlMenu() {
    pantalla_ = Pantalla::Menu;
    tieneObjetivo_ = false;
    reiniciarEfectosVisuales();
    actualizarMusicaFondo();
    update();
}

void MainWindow::reiniciarNivelActual() {
    if (pantalla_ != Pantalla::Jugando) {
        return;
    }

    ejecutarAccionSegura([this]() {
        juego_.reiniciarNivelActivo();
        mensajeError_.clear();
        tieneObjetivo_ = false;
        reiniciarEfectosVisuales();
        reloj_.restart();
        update();
    });
}

void MainWindow::manejarClickMenu(const QPointF& punto) {
    if (botonNivel1().contains(punto)) {
        iniciarNivelRutaTransmision();
    } else if (botonNivel2().contains(punto)) {
        iniciarNivelDefensaNucleo();
    } else if (botonDificultadFacil().contains(punto)) {
        juego_.establecerDificultadDefensa(DificultadDefensa::Facil);
        update();
    } else if (botonDificultadMedio().contains(punto)) {
        juego_.establecerDificultadDefensa(DificultadDefensa::Medio);
        update();
    } else if (botonDificultadDificil().contains(punto)) {
        juego_.establecerDificultadDefensa(DificultadDefensa::Dificil);
        update();
    } else if (botonSalir().contains(punto)) {
        close();
    }
}

void MainWindow::manejarClickJuego(const QPointF& punto) {
    if (juego_.nivelActivoFinalizado()) {
        return;
    }

    if (dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual()) != nullptr) {
        defenderEn(convertirDefensaALogica(punto));
    } else {
        lanzarDiscoHacia(convertirALogica(punto));
    }
    update();
}

void MainWindow::actualizarEfectosVisuales(std::chrono::milliseconds intervalo) {
    const auto consumir = [intervalo](std::chrono::milliseconds& pulso) {
        pulso = pulso > intervalo ? pulso - intervalo : std::chrono::milliseconds{0};
    };

    consumir(pulsoCheckpoint_);
    consumir(pulsoImpacto_);
    consumir(pulsoFinal_);

    if (const auto* nivelRuta = dynamic_cast<const NivelRutaTransmision*>(juego_.nivelActual())) {
        if (const Checkpoint* checkpoint = nivelRuta->checkpointActual()) {
            if (checkpoint->id() != ultimoCheckpointActivado_) {
                ultimoCheckpointActivado_ = checkpoint->id();
                posicionPulsoCheckpoint_ = checkpoint->posicion();
                pulsoCheckpoint_ = duracionPulsoCheckpoint;
                reproducirSonidoCheckpoint();
            }
        }
    }

    if (const auto* nivelDefensa = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual())) {
        const std::size_t amenazasActuales = nivelDefensa->discosActivos();
        if (amenazasActuales < ultimasAmenazasActivas_ && !nivelDefensa->estaFinalizado()) {
            posicionPulsoImpacto_ = nivelDefensa->posicionProyectilDefensor();
            pulsoImpacto_ = duracionPulsoImpacto;
            reproducirSonidoImpacto();
        }
        ultimasAmenazasActivas_ = amenazasActuales;
    }

    if (juego_.nivelActivoFinalizado() && !finalRegistrado_) {
        finalRegistrado_ = true;
        pulsoFinal_ = duracionPulsoFinal;
        if (juego_.nivelActivoVictoria()) {
            reproducirSonidoVictoria();
        } else if (juego_.nivelActivoDerrota()) {
            reproducirSonidoDerrota();
        }
    }
}

void MainWindow::reiniciarEfectosVisuales() {
    pulsoCheckpoint_ = std::chrono::milliseconds{0};
    pulsoImpacto_ = std::chrono::milliseconds{0};
    pulsoFinal_ = std::chrono::milliseconds{0};
    ultimoCheckpointActivado_.clear();
    if (const auto* nivelDefensa = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual())) {
        ultimasAmenazasActivas_ = nivelDefensa->discosActivos();
    } else {
        ultimasAmenazasActivas_ = 0;
    }
    finalRegistrado_ = false;
}

bool MainWindow::ejecutarAccionSegura(const std::function<void()>& accion) {
    try {
        accion();
        return true;
    } catch (const std::exception& error) {
        pantalla_ = Pantalla::Menu;
        actualizarMusicaFondo();
        registrarError(error);
        return false;
    }
}

void MainWindow::registrarError(const std::exception& error) {
    mensajeError_ = QString::fromLocal8Bit(error.what());
    tieneObjetivo_ = false;
    reiniciarEfectosVisuales();
    update();
}

void MainWindow::inicializarSonido() {
    musicaMenu_ = crearEfectoSonido(this, audioMenu, volumenMusicaMenu, QSoundEffect::Infinite);
    sonidoCheckpoint_ = crearEfectoSonido(this, audioCheckpoint, volumenCheckpoint);
    sonidoImpacto_ = crearEfectoSonido(this, audioImpacto, volumenImpacto);
    sonidoVictoria_ = crearEfectoSonido(this, audioVictoria, volumenResultado);
    sonidoDerrota_ = crearEfectoSonido(this, audioDerrota, volumenResultado);
}

void MainWindow::reproducirSonidoCheckpoint() {
    reproducirSonido(sonidoCheckpoint_);
}

void MainWindow::reproducirSonidoImpacto() {
    reproducirSonido(sonidoImpacto_);
}

void MainWindow::reproducirSonidoVictoria() {
    reproducirSonido(sonidoVictoria_);
}

void MainWindow::reproducirSonidoDerrota() {
    reproducirSonido(sonidoDerrota_);
}

void MainWindow::reproducirSonido(QSoundEffect* sonido) const {
    if (sonido != nullptr) {
        sonido->play();
    }
}

void MainWindow::actualizarMusicaFondo() {
    if (musicaMenu_ == nullptr) {
        return;
    }

    if (pantalla_ == Pantalla::Menu) {
        if (!musicaMenu_->isPlaying()) {
            musicaMenu_->play();
        }
    } else {
        musicaMenu_->stop();
    }
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

    nivel->dispararDefensa(objetivo);
}

void MainWindow::dibujarFondo(QPainter& painter) const {
    painter.fillRect(rect(), QColor{12, 16, 28});

    painter.setPen(QPen(QColor{30, 180, 220}, 1));
    for (int x = separacionGrid; x < width(); x += separacionGrid) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = separacionGrid; y < height(); y += separacionGrid) {
        painter.drawLine(0, y, width(), y);
    }
}

void MainWindow::dibujarMenu(QPainter& painter) const {
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor{5, 12, 24, 150});
    painter.drawRoundedRect(
        QRectF{width() / 2.0 - 250.0, 58.0, 500.0, 406.0},
        18.0,
        18.0);

    painter.setPen(QColor{220, 245, 255});
    painter.setFont(QFont{"Arial", 32, QFont::Bold});
    painter.drawText(QRectF{0.0, 76.0, static_cast<double>(width()), 54.0}, Qt::AlignCenter, "ULTIMATE EN TRON");
    painter.setPen(QPen(QColor{80, 240, 255}, 2));
    painter.drawLine(QPointF{width() / 2.0 - 150.0, 132.0}, QPointF{width() / 2.0 + 150.0, 132.0});

    painter.setFont(QFont{"Arial", 12});
    painter.setPen(QColor{170, 220, 235});
    painter.drawText(
        QRectF{0.0, 148.0, static_cast<double>(width()), 30.0},
        Qt::AlignCenter,
        "Selecciona una simulacion de combate");

    const QList<QPair<QRectF, QString>> botones{
        {botonNivel1(), "Jugar Nivel 1 - Ruta de Transmision"},
        {botonNivel2(), QString{"Jugar Nivel 2 - Defensa (%1)"}.arg(textoDificultad(juego_.dificultadDefensa()))},
        {botonSalir(), "Salir"}};

    for (const auto& boton : botones) {
        const bool hover = cursorSobreVentana_ && boton.first.contains(posicionCursor_);
        painter.setPen(QPen(hover ? QColor{140, 255, 255} : QColor{80, 220, 255}, hover ? 3 : 2));
        painter.setBrush(hover ? QColor{24, 58, 86} : QColor{18, 35, 58});
        painter.drawRoundedRect(boton.first, 8.0, 8.0);

        if (hover) {
            painter.setPen(QPen(QColor{130, 255, 250, 180}, 1));
            painter.drawRoundedRect(boton.first.adjusted(5.0, 5.0, -5.0, -5.0), 6.0, 6.0);
        }

        painter.setPen(hover ? QColor{255, 255, 255} : QColor{235, 250, 255});
        painter.setFont(QFont{"Arial", 12, QFont::Bold});
        painter.drawText(boton.first, Qt::AlignCenter, boton.second);
    }

    painter.setPen(QColor{170, 220, 235});
    painter.setFont(QFont{"Arial", 10, QFont::Bold});
    painter.drawText(
        QRectF{width() / 2.0 - 140.0, height() / 2.0 + 124.0, 280.0, 18.0},
        Qt::AlignCenter,
        "Dificultad del Nivel 2");

    const QList<QPair<QRectF, DificultadDefensa>> botonesDificultad{
        {botonDificultadFacil(), DificultadDefensa::Facil},
        {botonDificultadMedio(), DificultadDefensa::Medio},
        {botonDificultadDificil(), DificultadDefensa::Dificil}};
    for (const auto& boton : botonesDificultad) {
        const bool seleccionado = juego_.dificultadDefensa() == boton.second;
        const bool hover = cursorSobreVentana_ && boton.first.contains(posicionCursor_);
        painter.setPen(QPen(seleccionado ? QColor{255, 95, 140} : QColor{80, 220, 255}, seleccionado || hover ? 2 : 1));
        painter.setBrush(seleccionado
            ? QColor{60, 24, 48}
            : (hover ? QColor{24, 58, 86} : QColor{13, 28, 48}));
        painter.drawRoundedRect(boton.first, 7.0, 7.0);
        painter.setPen(QColor{235, 250, 255});
        painter.setFont(QFont{"Arial", 9, QFont::Bold});
        painter.drawText(boton.first, Qt::AlignCenter, textoDificultad(boton.second));
    }

    painter.setPen(QColor{160, 190, 210});
    painter.setFont(QFont{"Arial", 10});
    painter.drawText(
        QRectF{0.0, height() - 42.0, static_cast<double>(width()), 24.0},
        Qt::AlignCenter,
        "Atajos: 1 Nivel 1 | 2 Nivel 2 | Esc/M Menu | R Reiniciar");

    if (!mensajeError_.isEmpty()) {
        painter.setPen(QColor{255, 120, 130});
        painter.setFont(QFont{"Arial", 10, QFont::Bold});
        painter.drawText(
            QRectF{width() / 2.0 - 260.0, height() - 72.0, 520.0, 22.0},
            Qt::AlignCenter,
            QString{"Error de configuracion: %1"}.arg(mensajeError_));
    }
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
        if (pulsoCheckpoint_.count() > 0
            && checkpoint->posicion().distanciaA(posicionPulsoCheckpoint_) <= 0.01F) {
            const double progreso = progresoPulso(pulsoCheckpoint_, duracionPulsoCheckpoint);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(QColor{100, 255, 190, static_cast<int>(190 * (1.0 - progreso))}, 3));
            painter.drawEllipse(centro, 16.0 + progreso * 28.0, 16.0 + progreso * 28.0);
            painter.setPen(Qt::NoPen);
        }
        painter.setBrush(checkpoint->estaActivado()
            ? QColor{70, 230, 140}
            : QColor{240, 200, 70});
        painter.drawEllipse(centro, 12.0, 12.0);
        painter.setBrush(QColor{245, 255, 255});
        painter.drawEllipse(centro, 4.0, 4.0);
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
    painter.drawEllipse(jugador, 16.0, 16.0);
    painter.setBrush(QColor{220, 255, 245});
    painter.drawEllipse(jugador, 5.0, 5.0);

    const QPointF disco = convertirAPantalla(juego_.discoJugador().posicion());
    painter.setBrush(juego_.discoJugador().estaActivo()
        ? QColor{255, 255, 255}
        : QColor{130, 140, 150});
    painter.drawEllipse(disco, 8.0, 8.0);
    if (juego_.discoJugador().estaActivo()) {
        painter.setBrush(QColor{80, 240, 255, 160});
        painter.drawEllipse(disco, 14.0, 14.0);
    }
}

void MainWindow::dibujarNivelDefensaNucleo(QPainter& painter) {
    const auto* nivel = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual());
    if (nivel == nullptr) {
        return;
    }

    const QPointF jugador = convertirDefensaAPantalla(juego_.jugador().posicion());

    const QPointF horizonte{width() / 2.0, 92.0};
    painter.setPen(QPen(QColor{45, 130, 185}, 2));
    for (double carril : {-3.5, -1.75, 0.0, 1.75, 3.5}) {
        painter.drawLine(
            QPointF{width() / 2.0 + carril * 72.0, height() - 40.0},
            QPointF{horizonte.x() + carril * 9.0, horizonte.y()});
    }
    painter.setPen(QPen(QColor{35, 95, 145}, 1, Qt::DashLine));
    for (int i = 1; i <= 6; ++i) {
        const double t = i / 7.0;
        const double y = height() - 82.0 - t * (height() - 174.0);
        const double ancho = 260.0 * (1.0 - t) + 34.0 * t;
        painter.drawLine(QPointF{width() / 2.0 - ancho, y}, QPointF{width() / 2.0 + ancho, y});
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor{20, 36, 58, 180});
    painter.drawEllipse(jugador, 58.0, 22.0);
    painter.setBrush(QColor{40, 255, 210});
    painter.drawEllipse(jugador, 20.0, 20.0);
    painter.setBrush(QColor{230, 255, 245});
    painter.drawEllipse(jugador, 6.0, 6.0);

    std::vector<const DiscoEnemigo*> discosOrdenados;
    for (const DiscoEnemigo* disco : nivel->obtenerDiscosEnemigos()) {
        if (disco != nullptr) {
            discosOrdenados.push_back(disco);
        }
    }
    std::sort(
        discosOrdenados.begin(),
        discosOrdenados.end(),
        [](const DiscoEnemigo* izquierdo, const DiscoEnemigo* derecho) {
            return izquierdo->posicion().y() > derecho->posicion().y();
        });

    for (const DiscoEnemigo* disco : discosOrdenados) {
        if (disco == nullptr) {
            continue;
        }

        const QPointF centro = convertirDefensaAPantalla(disco->posicion());
        const double escala = escalaDefensa(disco->posicion());
        const double radio = 9.0 * escala;
        painter.setPen(QPen(QColor{255, 150, 120, 170}, 2));
        painter.drawLine(centro, jugador);
        painter.setPen(Qt::NoPen);
        painter.setBrush(disco->estaDestruido()
            ? QColor{70, 80, 90}
            : QColor{255, 80, 90});
        painter.drawEllipse(centro, radio, radio);
        if (!disco->estaDestruido()) {
            painter.setBrush(QColor{255, 70, 80, 90});
            painter.drawEllipse(centro, radio * 1.65, radio * 1.65);
            painter.setBrush(QColor{255, 230, 170});
            painter.drawEllipse(centro, radio * 0.35, radio * 0.35);
        }
    }

    if (nivel->proyectilDefensorActivo()) {
        const QPointF proyectil = convertirDefensaAPantalla(nivel->posicionProyectilDefensor());
        const double escala = escalaDefensa(nivel->posicionProyectilDefensor());
        painter.setPen(QPen(QColor{80, 255, 235}, 3));
        painter.drawLine(jugador, proyectil);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor{110, 255, 245});
        painter.drawEllipse(proyectil, 6.0 * escala, 6.0 * escala);
        painter.setBrush(QColor{110, 255, 245, 95});
        painter.drawEllipse(proyectil, 13.0 * escala, 13.0 * escala);
    }

    if (pulsoImpacto_.count() > 0) {
        const double progreso = progresoPulso(pulsoImpacto_, duracionPulsoImpacto);
        const QPointF centro = convertirDefensaAPantalla(posicionPulsoImpacto_);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor{120, 255, 235, static_cast<int>(220 * (1.0 - progreso))}, 3));
        painter.drawEllipse(centro, 10.0 + progreso * 36.0, 10.0 + progreso * 36.0);
        painter.setPen(Qt::NoPen);
    }

}

void MainWindow::dibujarEstado(QPainter& painter) const {
    const QRectF panel{18.0, 18.0, 310.0, 154.0};
    const bool defensa = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual()) != nullptr;
    const QColor acentoHud = defensa ? QColor{255, 95, 140} : QColor{80, 240, 255};
    painter.setPen(QPen(acentoHud, 1));
    painter.setBrush(QColor{7, 18, 32, 210});
    painter.drawRoundedRect(panel, 10.0, 10.0);

    painter.setPen(acentoHud);
    painter.setFont(QFont{"Arial", 16, QFont::Bold});
    painter.drawText(QRectF{34.0, 28.0, 280.0, 24.0}, Qt::AlignLeft, "ULTIMATE EN TRON");

    QString nivelTexto = "Nivel: desconocido";
    QString tiempoTexto = "Tiempo: --";
    QString objetivoTexto = "Objetivo: --";

    if (const auto* nivelDefensa = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual())) {
        nivelTexto = QString{"Nivel: Defensa del Nucleo (%1)"}.arg(textoDificultad(nivelDefensa->dificultad()));
        tiempoTexto = QString{"Tiempo: %1 s"}.arg(nivelDefensa->tiempoRestante().count() / 1000.0, 0, 'f', 1);
        objetivoTexto = QString{"Amenazas activas: %1"}.arg(nivelDefensa->discosActivos());
    } else {
        const auto* nivelRuta = dynamic_cast<const NivelRutaTransmision*>(juego_.nivelActual());
        nivelTexto = "Nivel: Ruta de Transmision";
        if (nivelRuta != nullptr) {
            const Checkpoint* objetivo = nivelRuta->objetivoActualCheckpoint();
            const QString checkpoint = objetivo != nullptr
                ? QString::fromStdString(objetivo->id())
                : QString{"sin objetivo"};
            objetivoTexto = QString{"Objetivo: %1"}.arg(checkpoint);
            tiempoTexto = QString{"Tiempo: %1 s"}.arg(
                nivelRuta->tiempoRestanteCheckpoint().count() / 1000.0,
                0,
                'f',
                1);
        }
    }

    QString estadoTexto = "Estado: en progreso";
    if (!juego_.nivelActivoFinalizado()) {
        estadoTexto = "Estado: en progreso";
    } else if (juego_.nivelActivoVictoria()) {
        estadoTexto = "Estado: victoria";
    } else if (juego_.nivelActivoDerrota()) {
        estadoTexto = "Estado: derrota";
    } else {
        estadoTexto = "Estado: finalizado";
    }

    painter.setFont(QFont{"Arial", 10});
    painter.setPen(QColor{220, 240, 255});
    painter.drawText(34, 66, nivelTexto);
    painter.drawText(34, 88, tiempoTexto);
    painter.drawText(34, 110, objetivoTexto);
    painter.drawText(34, 132, estadoTexto);

    painter.setPen(QColor{150, 205, 225});
    painter.drawText(34, 154, "Clic: accion | R: reiniciar | Esc/M: menu");
}

void MainWindow::dibujarOverlayFinal(QPainter& painter) const {
    if (!juego_.nivelActivoFinalizado()) {
        return;
    }

    const bool victoria = juego_.nivelActivoVictoria();
    const QColor acento = victoria ? QColor{90, 255, 170} : QColor{255, 95, 105};
    const QString titulo = victoria ? "VICTORIA" : "DERROTA";
    const QString subtitulo = victoria
        ? "Sistema estabilizado. Ruta completada."
        : "Conexion perdida. Reintenta la secuencia.";

    const double pulso = pulsoFinal_.count() > 0
        ? std::sin(progresoPulso(pulsoFinal_, duracionPulsoFinal) * 3.14159265358979323846)
        : 0.0;

    painter.setBrush(QColor{3, 8, 18, 188});
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    const QRectF tarjeta{width() / 2.0 - 230.0, height() / 2.0 - 86.0, 460.0, 172.0};
    painter.setBrush(QColor{7, 18, 32, 235});
    painter.setPen(QPen(acento, 2 + pulso * 2));
    painter.drawRoundedRect(tarjeta, 14.0, 14.0);

    painter.setPen(acento);
    painter.setFont(QFont{"Arial", 28, QFont::Bold});
    painter.drawText(
        QRectF{tarjeta.x() + 20.0, tarjeta.y() + 26.0, tarjeta.width() - 40.0, 42.0},
        Qt::AlignCenter,
        titulo);

    painter.setPen(QColor{220, 240, 255});
    painter.setFont(QFont{"Arial", 11});
    painter.drawText(
        QRectF{tarjeta.x() + 28.0, tarjeta.y() + 78.0, tarjeta.width() - 56.0, 28.0},
        Qt::AlignCenter,
        subtitulo);

    painter.setPen(QColor{150, 225, 245});
    painter.setFont(QFont{"Arial", 10, QFont::Bold});
    painter.drawText(
        QRectF{tarjeta.x() + 28.0, tarjeta.y() + 118.0, tarjeta.width() - 56.0, 28.0},
        Qt::AlignCenter,
        "R para reiniciar | Esc o M para volver al menu");
}
