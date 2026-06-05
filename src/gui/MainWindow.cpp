#include "gui/MainWindow.h"

#include "logic/BarreraEstatica.h"
#include "logic/BarreraMovil.h"
#include "logic/BarreraTemporizada.h"
#include "logic/NivelDefensaNucleo.h"
#include "logic/NivelRutaTransmision.h"
#include "logic/SuperficieRebote.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QPolygonF>
#include <QString>
#include <QSoundEffect>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <array>
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

constexpr double escalaRuta = 21.5;
constexpr double origenRutaX = 36.0;
constexpr double profundidadMaximaDefensa = 10.5;
constexpr double margenInferiorDefensa = 46.0;
constexpr double horizonteDefensaY = 86.0;
constexpr double anchoCarrilDefensa = 22.0;
constexpr double variacionCarrilDefensa = 78.0;

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

enum class SpriteSheet {
    Nivel1,
    Nivel2
};

enum class SpriteId {
    JugadorRuta,
    DiscoJugador,
    Checkpoint,
    Nucleo,
    DronAzul,
    DronRojo,
    CompuertaVertical,
    CompuertaHorizontal,
    PanelDelgado,
    ReboteMagenta,
    PulsoCyan,
    ExplosionCyan,
    ExplosionRoja,
    JugadorDefensa,
    DiscoEnemigo,
    ProyectilDefensor,
    Reticula,
    CarrilDefensa,
    BotonNeon,
    Victoria,
    Derrota,
    BaseSombra
};

constexpr auto duracionPulsoCheckpoint = std::chrono::milliseconds{620};
constexpr auto duracionPulsoImpacto = std::chrono::milliseconds{460};
constexpr auto duracionPulsoFinal = std::chrono::milliseconds{1200};

QPixmap& hojaSprites(SpriteSheet hoja) {
    static QPixmap hojaNivel1{":/sprites/sheet_level1.png"};
    static QPixmap hojaNivel2{":/sprites/sheet_level2.png"};
    return hoja == SpriteSheet::Nivel1 ? hojaNivel1 : hojaNivel2;
}

QRectF celdaSprite(SpriteSheet hoja, int columna, int fila) {
    const QPixmap& pixmap = hojaSprites(hoja);
    if (pixmap.isNull()) {
        return QRectF{};
    }

    constexpr int columnas = 4;
    constexpr int filas = 3;
    const double ancho = pixmap.width() / static_cast<double>(columnas);
    const double alto = pixmap.height() / static_cast<double>(filas);
    const double margenX = ancho * 0.08;
    const double margenY = alto * 0.08;
    return QRectF{
        columna * ancho + margenX,
        fila * alto + margenY,
        ancho - margenX * 2.0,
        alto - margenY * 2.0};
}

SpriteSheet hojaDeSprite(SpriteId sprite) {
    switch (sprite) {
    case SpriteId::JugadorRuta:
    case SpriteId::DiscoJugador:
    case SpriteId::DiscoEnemigo:
    case SpriteId::ExplosionCyan:
    case SpriteId::ExplosionRoja:
    case SpriteId::Reticula:
    case SpriteId::CarrilDefensa:
    case SpriteId::BotonNeon:
    case SpriteId::Victoria:
    case SpriteId::Derrota:
    case SpriteId::BaseSombra:
        return SpriteSheet::Nivel1;
    default:
        return SpriteSheet::Nivel2;
    }
}

QRectF recorteSprite(SpriteId sprite) {
    const SpriteSheet hoja = hojaDeSprite(sprite);
    switch (sprite) {
    case SpriteId::JugadorRuta:
        return celdaSprite(hoja, 0, 0);
    case SpriteId::DiscoJugador:
        return celdaSprite(hoja, 2, 0);
    case SpriteId::ProyectilDefensor:
        return celdaSprite(hoja, 1, 0);
    case SpriteId::Checkpoint:
        return celdaSprite(hoja, 2, 0);
    case SpriteId::Nucleo:
        return celdaSprite(hoja, 3, 0);
    case SpriteId::DronAzul:
        return celdaSprite(hoja, 0, 1);
    case SpriteId::DronRojo:
        return celdaSprite(hoja, 1, 1);
    case SpriteId::CompuertaVertical:
        return celdaSprite(hoja, 2, 1);
    case SpriteId::CompuertaHorizontal:
        return celdaSprite(hoja, 3, 1);
    case SpriteId::PanelDelgado:
        return celdaSprite(hoja, 0, 2);
    case SpriteId::ReboteMagenta:
        return celdaSprite(hoja, 1, 2);
    case SpriteId::PulsoCyan:
        return celdaSprite(hoja, 3, 2);
    case SpriteId::ExplosionCyan:
        return celdaSprite(hoja, 0, 1);
    case SpriteId::ExplosionRoja:
        return celdaSprite(hoja, 1, 1);
    case SpriteId::JugadorDefensa:
        return celdaSprite(hoja, 0, 0);
    case SpriteId::DiscoEnemigo:
        return celdaSprite(SpriteSheet::Nivel1, 1, 0);
    case SpriteId::Reticula:
        return celdaSprite(hoja, 2, 1);
    case SpriteId::CarrilDefensa:
        return celdaSprite(hoja, 3, 2);
    case SpriteId::BotonNeon:
        return celdaSprite(hoja, 3, 1);
    case SpriteId::Victoria:
        return celdaSprite(hoja, 0, 2);
    case SpriteId::Derrota:
        return celdaSprite(hoja, 1, 2);
    case SpriteId::BaseSombra:
        return celdaSprite(hoja, 2, 2);
    }

    return QRectF{};
}

bool dibujarSpriteEnRect(QPainter& painter, SpriteId sprite, const QRectF& destino) {
    QPixmap& hoja = hojaSprites(hojaDeSprite(sprite));
    const QRectF origen = recorteSprite(sprite);
    if (hoja.isNull() || origen.isNull() || destino.isNull()) {
        return false;
    }

    const double escala = std::min(destino.width() / origen.width(), destino.height() / origen.height());
    const QSizeF ajustado{origen.width() * escala, origen.height() * escala};
    const QRectF destinoAjustado{
        destino.center().x() - ajustado.width() / 2.0,
        destino.center().y() - ajustado.height() / 2.0,
        ajustado.width(),
        ajustado.height()};
    painter.drawPixmap(destinoAjustado, hoja, origen);
    return true;
}

bool dibujarSpriteCentrado(QPainter& painter, SpriteId sprite, const QPointF& centro, const QSizeF& tamano) {
    return dibujarSpriteEnRect(
        painter,
        sprite,
        QRectF{centro.x() - tamano.width() / 2.0, centro.y() - tamano.height() / 2.0, tamano.width(), tamano.height()});
}

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
    if (trazandoDisparo_) {
        ultimoObjetivo_ = convertirALogica(event->position());
        tieneObjetivo_ = true;
    }
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

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && trazandoDisparo_) {
        ultimoObjetivo_ = convertirALogica(event->position());
        lanzarDiscoHacia(ultimoObjetivo_);
        trazandoDisparo_ = false;
        update();
    }

    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QMainWindow::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
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
    constexpr double escalaMinimaFondo = 0.26;
    constexpr double escalaMaximaCerca = 2.05;
    const double profundidad = std::clamp(static_cast<double>(posicion.y()) / profundidadMaximaDefensa, 0.0, 1.0);
    const double cercania = 1.0 - profundidad;
    const double cercaniaSuave = cercania * cercania * (3.0 - 2.0 * cercania);
    return escalaMinimaFondo + cercaniaSuave * (escalaMaximaCerca - escalaMinimaFondo);
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
        trazandoDisparo_ = false;
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
        trazandoDisparo_ = false;
        reiniciarEfectosVisuales();
        reloj_.restart();
        actualizarMusicaFondo();
        update();
    });
}

void MainWindow::volverAlMenu() {
    pantalla_ = Pantalla::Menu;
    tieneObjetivo_ = false;
    trazandoDisparo_ = false;
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
        trazandoDisparo_ = false;
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
        ultimoObjetivo_ = convertirALogica(punto);
        tieneObjetivo_ = true;
        trazandoDisparo_ = true;
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
    trazandoDisparo_ = false;
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
    juego_.discoJugador().lanzarDesde(origen, direccion, 4.2F);
}

void MainWindow::defenderEn(const Posicion& objetivo) {
    auto* nivel = dynamic_cast<NivelDefensaNucleo*>(juego_.nivelActual());
    if (nivel == nullptr) {
        return;
    }

    nivel->dispararDefensa(objetivo);
}

void MainWindow::dibujarFondo(QPainter& painter) const {
    const bool defensa = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual()) != nullptr;
    painter.fillRect(rect(), defensa ? QColor{5, 10, 20} : QColor{12, 16, 28});

    if (defensa) {
        painter.setPen(QPen(QColor{10, 44, 72, 70}, 1));
        const int horizonte = static_cast<int>(horizonteDefensaY);
        for (int y = horizonte; y < height(); y += separacionGrid * 2) {
            painter.drawLine(0, y, width(), y);
        }
        painter.setPen(QPen(QColor{28, 115, 170, 120}, 1));
        painter.drawLine(0, horizonte, width(), horizonte);
        return;
    }

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

    const auto rectLogico = [this](double x1, double y1, double x2, double y2) {
        const QPointF superiorIzquierda = convertirAPantalla(Posicion{
            static_cast<float>(x1),
            static_cast<float>(y2)});
        const QPointF inferiorDerecha = convertirAPantalla(Posicion{
            static_cast<float>(x2),
            static_cast<float>(y1)});
        return QRectF{superiorIzquierda, inferiorDerecha}.normalized();
    };

    const QRectF carrilSuperior = rectLogico(0.25, 1.45, 18.1, 4.55);
    painter.setPen(QPen(QColor{80, 240, 255, 120}, 2));
    painter.setBrush(QColor{20, 80, 105, 55});
    painter.drawRoundedRect(carrilSuperior, 12.0, 12.0);
    painter.setPen(QColor{130, 245, 255, 150});
    painter.setFont(QFont{"Arial", 8, QFont::Bold});
    painter.drawText(
        carrilSuperior.adjusted(8.0, 5.0, -8.0, -5.0),
        Qt::AlignTop | Qt::AlignLeft,
        "TRAMOS A / C");

    const QRectF carrilInferior = rectLogico(5.9, -3.85, 39.45, -0.65);
    painter.setPen(QPen(QColor{255, 90, 205, 105}, 2));
    painter.setBrush(QColor{92, 24, 86, 58});
    painter.drawRoundedRect(carrilInferior, 12.0, 12.0);
    painter.setPen(QColor{255, 175, 235, 150});
    painter.setFont(QFont{"Arial", 8, QFont::Bold});
    painter.drawText(
        carrilInferior.adjusted(8.0, 5.0, -8.0, -5.0),
        Qt::AlignTop | Qt::AlignLeft,
        "TRAMOS B / E / F");

    const auto dibujarZona = [&painter, &rectLogico](const TramoRutaTransmision& tramo, const QColor& color) {
        const QRectF zona = rectLogico(tramo.inicioX, tramo.minimoY, tramo.finX, tramo.maximoY);
        painter.setPen(QPen(color, 1));
        painter.setBrush(QColor{color.red(), color.green(), color.blue(), 30});
        painter.drawRoundedRect(zona, 10.0, 10.0);
        painter.setPen(QColor{color.red(), color.green(), color.blue(), 150});
        painter.setFont(QFont{"Arial", 8, QFont::Bold});
        painter.drawText(
            zona.adjusted(6.0, 4.0, -6.0, -4.0),
            Qt::AlignTop | Qt::AlignHCenter,
            QString::fromStdString(tramo.etiqueta));
    };

    static const std::array<QColor, 6> coloresTramos{
        QColor{90, 255, 235},
        QColor{100, 190, 255},
        QColor{255, 190, 80},
        QColor{120, 230, 255},
        QColor{255, 80, 210},
        QColor{255, 80, 105}};
    std::size_t indiceTramo = 0;
    for (const TramoRutaTransmision& tramo : nivel->obtenerTramos()) {
        dibujarZona(tramo, coloresTramos[indiceTramo % coloresTramos.size()]);
        ++indiceTramo;
    }

    QPolygonF rutaSugerida;
    rutaSugerida << convertirAPantalla(Posicion{0.0F, 0.0F});
    for (const Checkpoint* checkpoint : nivel->obtenerCheckpoints()) {
        if (checkpoint != nullptr) {
            rutaSugerida << convertirAPantalla(checkpoint->posicion());
        }
    }
    if (const NodoCentralEnergia* meta = nivel->metaFinal()) {
        rutaSugerida << convertirAPantalla(meta->posicion());
    }
    painter.setPen(QPen(QColor{75, 210, 240, 70}, 1, Qt::DashLine));
    painter.drawPolyline(rutaSugerida);

    painter.setPen(Qt::NoPen);

    const Checkpoint* objetivoActual = nivel->objetivoActualCheckpoint();
    int indiceCheckpoint = 1;
    for (const Checkpoint* checkpoint : nivel->obtenerCheckpoints()) {
        if (checkpoint == nullptr) {
            continue;
        }

        const QPointF centro = convertirAPantalla(checkpoint->posicion());
        const bool esObjetivo = checkpoint == objetivoActual;
        const bool esCheckpointUno = checkpoint->id() == "checkpoint-1";
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(esObjetivo ? QColor{120, 255, 210} : QColor{255, 225, 95, 135}, esObjetivo ? 3 : 1));
        painter.drawEllipse(centro, esCheckpointUno ? 22.0 : 18.0, esCheckpointUno ? 22.0 : 18.0);
        painter.setPen(Qt::NoPen);

        if (pulsoCheckpoint_.count() > 0
            && checkpoint->posicion().distanciaA(posicionPulsoCheckpoint_) <= 0.01F) {
            const double progreso = progresoPulso(pulsoCheckpoint_, duracionPulsoCheckpoint);
            if (!dibujarSpriteCentrado(painter, SpriteId::PulsoCyan, centro, QSizeF{60.0 + progreso * 42.0, 48.0 + progreso * 34.0})) {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(QColor{100, 255, 190, static_cast<int>(190 * (1.0 - progreso))}, 3));
                painter.drawEllipse(centro, 16.0 + progreso * 28.0, 16.0 + progreso * 28.0);
                painter.setPen(Qt::NoPen);
            }
        }
        if (!dibujarSpriteCentrado(painter, SpriteId::Checkpoint, centro, QSizeF{44.0, 44.0})) {
            painter.setBrush(checkpoint->estaActivado()
                ? QColor{70, 230, 140}
                : QColor{240, 200, 70});
            painter.drawEllipse(centro, 12.0, 12.0);
            painter.setBrush(QColor{245, 255, 255});
            painter.drawEllipse(centro, 4.0, 4.0);
        }
        painter.setPen(QColor{8, 20, 30});
        painter.setFont(QFont{"Arial", 8, QFont::Bold});
        painter.drawText(QRectF{centro.x() - 8.0, centro.y() - 8.0, 16.0, 16.0}, Qt::AlignCenter, QString::number(indiceCheckpoint));
        painter.setPen(Qt::NoPen);
        ++indiceCheckpoint;
    }

    if (const NodoCentralEnergia* meta = nivel->metaFinal()) {
        const QPointF centro = convertirAPantalla(meta->posicion());
        if (!dibujarSpriteCentrado(painter, SpriteId::Nucleo, centro, QSizeF{74.0, 74.0})) {
            QPolygonF rombo;
            rombo << QPointF{centro.x(), centro.y() - 18.0}
                  << QPointF{centro.x() + 18.0, centro.y()}
                  << QPointF{centro.x(), centro.y() + 18.0}
                  << QPointF{centro.x() - 18.0, centro.y()};
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(QColor{210, 120, 255, 120}, 4));
            painter.drawEllipse(centro, 30.0, 30.0);
            painter.setPen(QPen(QColor{235, 200, 255}, 2));
            painter.setBrush(QColor{180, 90, 255});
            painter.drawPolygon(rombo);
        }
        painter.setPen(QColor{245, 230, 255});
        painter.setFont(QFont{"Arial", 8, QFont::Bold});
        painter.drawText(QRectF{centro.x() - 32.0, centro.y() + 22.0, 64.0, 16.0}, Qt::AlignCenter, "NUCLEO");
        painter.setPen(Qt::NoPen);
    }

    for (const Obstaculo* obstaculo : nivel->obtenerObstaculos()) {
        if (const auto* barrera = dynamic_cast<const BarreraEstatica*>(obstaculo)) {
            const QPointF centro = convertirAPantalla(barrera->posicion());
            if (!dibujarSpriteCentrado(painter, SpriteId::PanelDelgado, centro, QSizeF{16.0, 42.0})) {
                painter.setBrush(QColor{230, 70, 70});
                painter.drawRoundedRect(QRectF{centro.x() - 5.0, centro.y() - 16.0, 10.0, 32.0}, 3.0, 3.0);
            }
        } else if (const auto* barrera = dynamic_cast<const BarreraMovil*>(obstaculo)) {
            const QPointF centro = convertirAPantalla(barrera->posicion());
            const bool compuerta = barrera->posicion().x() < 3.2F;
            const bool puertaHorizontal =
                barrera->posicion().x() > 12.8F && barrera->posicion().x() < 18.35F
                && barrera->posicion().y() > 0.15F && barrera->posicion().y() < 1.4F;
            const QColor colorBase = compuerta ? QColor{85, 255, 230} : QColor{95, 210, 255};
            if (compuerta) {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(QColor{85, 255, 230, 115}, 2));
                painter.drawLine(QPointF{centro.x() - 26.0, centro.y() - 30.0}, QPointF{centro.x() - 26.0, centro.y() + 30.0});
                painter.drawLine(QPointF{centro.x() + 26.0, centro.y() - 30.0}, QPointF{centro.x() + 26.0, centro.y() + 30.0});
                painter.setPen(Qt::NoPen);
            }
            if (!dibujarSpriteCentrado(
                    painter,
                    puertaHorizontal ? SpriteId::CompuertaHorizontal : SpriteId::CompuertaVertical,
                    centro,
                    puertaHorizontal ? QSizeF{74.0, 30.0} : QSizeF{42.0, 74.0})) {
                painter.setBrush(colorBase);
                painter.drawRoundedRect(puertaHorizontal
                    ? QRectF{centro.x() - 32.0, centro.y() - 11.0, 64.0, 22.0}
                    : QRectF{centro.x() - 18.0, centro.y() - 26.0, 36.0, 52.0},
                    6.0,
                    6.0);
                painter.setBrush(QColor{225, 255, 255});
                painter.drawRect(puertaHorizontal
                    ? QRectF{centro.x() - 26.0, centro.y() - 3.0, 52.0, 6.0}
                    : QRectF{centro.x() - 4.0, centro.y() - 20.0, 8.0, 40.0});
            }
        } else if (const auto* barrera = dynamic_cast<const BarreraTemporizada*>(obstaculo)) {
            const QPointF centro = convertirAPantalla(barrera->posicion());
            painter.setBrush(barrera->estaActivo()
                ? QColor{255, 120, 70}
                : QColor{70, 90, 120});
            painter.drawRect(QRectF{centro.x() - 14.0, centro.y() - 28.0, 28.0, 56.0});
        } else if (const auto* rebote = dynamic_cast<const SuperficieRebote*>(obstaculo)) {
            const QPointF centro = convertirAPantalla(rebote->posicion());
            const bool paredVertical = std::abs(rebote->normal().x()) > std::abs(rebote->normal().y());
            if (!dibujarSpriteCentrado(painter, SpriteId::ReboteMagenta, centro, paredVertical ? QSizeF{34.0, 82.0} : QSizeF{82.0, 34.0})) {
                painter.setBrush(QColor{255, 60, 210, 70});
                painter.drawEllipse(centro, paredVertical ? 18.0 : 30.0, paredVertical ? 30.0 : 18.0);
                painter.setBrush(QColor{255, 80, 210});
                painter.drawRoundedRect(paredVertical
                    ? QRectF{centro.x() - 9.0, centro.y() - 34.0, 18.0, 68.0}
                    : QRectF{centro.x() - 28.0, centro.y() - 9.0, 56.0, 18.0},
                    8.0,
                    8.0);
                painter.setBrush(QColor{255, 190, 240});
                painter.drawEllipse(centro, 5.0, 5.0);
            }
            const QPointF normal = convertirAPantalla(Posicion{
                rebote->posicion().x() + rebote->normal().x() * 0.55F,
                rebote->posicion().y() + rebote->normal().y() * 0.55F});
            painter.setPen(QPen(QColor{255, 220, 245}, 2));
            painter.drawLine(centro, normal);
            painter.setPen(Qt::NoPen);
        }
    }

    for (const Dron* dron : nivel->obtenerDrones()) {
        if (dron == nullptr) {
            continue;
        }

        const QPointF centro = convertirAPantalla(dron->posicion());
        const bool dronFinal = dron->posicion().x() > 20.0F;
        const auto& ruta = dron->rutaVigilancia();
        if (ruta.size() >= 2) {
            QPolygonF patrulla;
            for (const Posicion& punto : ruta) {
                patrulla << convertirAPantalla(punto);
            }
            painter.setPen(QPen(dronFinal ? QColor{255, 80, 95, 120} : QColor{80, 180, 255, 90}, dronFinal ? 2 : 1, Qt::DashLine));
            painter.drawPolyline(patrulla);
            painter.setPen(Qt::NoPen);
        }

        if (!dibujarSpriteCentrado(painter, dronFinal ? SpriteId::DronRojo : SpriteId::DronAzul, centro, dronFinal ? QSizeF{54.0, 54.0} : QSizeF{42.0, 42.0})) {
            painter.setBrush(dronFinal ? QColor{255, 60, 70, 85} : QColor{80, 180, 255, 65});
            painter.drawEllipse(centro, dronFinal ? 24.0 : 17.0, dronFinal ? 24.0 : 17.0);
            painter.setBrush(dronFinal ? QColor{255, 60, 70} : QColor{80, 180, 255});
            painter.drawEllipse(centro, dronFinal ? 14.0 : 10.5, dronFinal ? 14.0 : 10.5);
        }
        if (dronFinal) {
            painter.setPen(QColor{255, 180, 180});
            painter.setFont(QFont{"Arial", 8, QFont::Bold});
            painter.drawText(QRectF{centro.x() - 34.0, centro.y() - 33.0, 68.0, 14.0}, Qt::AlignCenter, "DEFENSOR");
            painter.setPen(Qt::NoPen);
        }
    }

    const QPointF jugador = convertirAPantalla(juego_.jugador().posicion());
    if (trazandoDisparo_) {
        const QPointF objetivo = convertirAPantalla(ultimoObjetivo_);
        painter.setPen(QPen(QColor{120, 240, 255}, 2, Qt::DashLine));
        painter.drawLine(jugador, objetivo);
        painter.setBrush(QColor{120, 240, 255, 95});
        painter.drawEllipse(objetivo, 7.0, 7.0);
        painter.setFont(QFont{"Arial", 8, QFont::Bold});
        painter.setPen(QColor{170, 250, 255});
        painter.drawText(QRectF{objetivo.x() + 8.0, objetivo.y() - 18.0, 92.0, 16.0}, Qt::AlignLeft, "TRAYECTORIA");
        painter.setPen(Qt::NoPen);
    }

    if (!dibujarSpriteCentrado(painter, SpriteId::JugadorRuta, jugador, QSizeF{64.0, 82.0})) {
        painter.setBrush(QColor{40, 255, 210});
        painter.drawEllipse(jugador, 16.0, 16.0);
        painter.setBrush(QColor{220, 255, 245});
        painter.drawEllipse(jugador, 5.0, 5.0);
    }

    const QPointF disco = convertirAPantalla(juego_.discoJugador().posicion());
    if (!dibujarSpriteCentrado(painter, SpriteId::DiscoJugador, disco, juego_.discoJugador().estaActivo() ? QSizeF{34.0, 26.0} : QSizeF{24.0, 18.0})) {
        painter.setBrush(juego_.discoJugador().estaActivo()
            ? QColor{255, 255, 255}
            : QColor{130, 140, 150});
        painter.drawEllipse(disco, 8.0, 8.0);
        if (juego_.discoJugador().estaActivo()) {
            painter.setBrush(QColor{80, 240, 255, 160});
            painter.drawEllipse(disco, 14.0, 14.0);
        }
    }
}

void MainWindow::dibujarNivelDefensaNucleo(QPainter& painter) {
    const auto* nivel = dynamic_cast<const NivelDefensaNucleo*>(juego_.nivelActual());
    if (nivel == nullptr) {
        return;
    }

    const QPointF jugador = convertirDefensaAPantalla(juego_.jugador().posicion());

    const QPointF horizonte{width() / 2.0, horizonteDefensaY};
    const double fondoY = height() - margenInferiorDefensa;
    const double anchoCerca = (anchoCarrilDefensa + variacionCarrilDefensa) * 1.18;
    const double anchoLejos = anchoCarrilDefensa * 0.24;

    QPolygonF pista;
    pista << QPointF{width() / 2.0 - anchoCerca * 4.55, fondoY + 26.0}
          << QPointF{width() / 2.0 + anchoCerca * 4.55, fondoY + 26.0}
          << QPointF{horizonte.x() + anchoLejos * 4.55, horizonte.y()}
          << QPointF{horizonte.x() - anchoLejos * 4.55, horizonte.y()};
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor{7, 38, 64, 118});
    painter.drawPolygon(pista);

    QPolygonF nucleoCentral;
    nucleoCentral << QPointF{width() / 2.0 - anchoCerca * 1.3, fondoY + 26.0}
                  << QPointF{width() / 2.0 + anchoCerca * 1.3, fondoY + 26.0}
                  << QPointF{horizonte.x() + anchoLejos * 1.3, horizonte.y()}
                  << QPointF{horizonte.x() - anchoLejos * 1.3, horizonte.y()};
    painter.setBrush(QColor{20, 160, 210, 48});
    painter.drawPolygon(nucleoCentral);

    QPolygonF lateralIzquierdo;
    lateralIzquierdo << QPointF{width() / 2.0 - anchoCerca * 4.55, fondoY + 26.0}
                     << QPointF{width() / 2.0 - anchoCerca * 2.15, fondoY + 26.0}
                     << QPointF{horizonte.x() - anchoLejos * 2.15, horizonte.y()}
                     << QPointF{horizonte.x() - anchoLejos * 4.55, horizonte.y()};
    QPolygonF lateralDerecho;
    lateralDerecho << QPointF{width() / 2.0 + anchoCerca * 2.15, fondoY + 26.0}
                   << QPointF{width() / 2.0 + anchoCerca * 4.55, fondoY + 26.0}
                   << QPointF{horizonte.x() + anchoLejos * 4.55, horizonte.y()}
                   << QPointF{horizonte.x() + anchoLejos * 2.15, horizonte.y()};
    painter.setBrush(QColor{4, 22, 42, 105});
    painter.drawPolygon(lateralIzquierdo);
    painter.drawPolygon(lateralDerecho);

    for (int i = 0; i < 9; ++i) {
        const double u0 = i / 9.0;
        const double u1 = (i + 1) / 9.0;
        const double t0 = 1.0 - std::pow(1.0 - u0, 1.75);
        const double t1 = 1.0 - std::pow(1.0 - u1, 1.75);
        const double y0 = fondoY - t0 * (fondoY - horizonte.y());
        const double y1 = fondoY - t1 * (fondoY - horizonte.y());
        const double ancho0 = anchoCerca * 4.1 * (1.0 - t0) + anchoLejos * 4.1 * t0;
        const double ancho1 = anchoCerca * 4.1 * (1.0 - t1) + anchoLejos * 4.1 * t1;
        QPolygonF banda;
        banda << QPointF{width() / 2.0 - ancho0, y0}
              << QPointF{width() / 2.0 + ancho0, y0}
              << QPointF{width() / 2.0 + ancho1, y1}
              << QPointF{width() / 2.0 - ancho1, y1};
        painter.setBrush(i % 2 == 0 ? QColor{20, 145, 190, 30} : QColor{2, 16, 32, 58});
        painter.drawPolygon(banda);
    }

    painter.setPen(QPen(QColor{60, 190, 245}, 2));
    for (double carril : {-3.75, -2.25, -1.05, 0.0, 1.05, 2.25, 3.75}) {
        painter.drawLine(
            QPointF{width() / 2.0 + carril * anchoCerca, fondoY + 20.0},
            QPointF{horizonte.x() + carril * anchoLejos, horizonte.y()});
    }
    painter.setPen(QPen(QColor{90, 235, 255, 150}, 5));
    painter.drawLine(QPointF{width() / 2.0 - anchoCerca * 4.55, fondoY + 26.0}, QPointF{horizonte.x() - anchoLejos * 4.55, horizonte.y()});
    painter.drawLine(QPointF{width() / 2.0 + anchoCerca * 4.55, fondoY + 26.0}, QPointF{horizonte.x() + anchoLejos * 4.55, horizonte.y()});
    painter.setPen(QPen(QColor{120, 255, 255, 155}, 3));
    painter.drawLine(QPointF{width() / 2.0, fondoY + 26.0}, horizonte);
    painter.setPen(QPen(QColor{65, 155, 205, 145}, 1, Qt::DashLine));
    for (int i = 1; i <= 11; ++i) {
        const double u = i / 12.0;
        const double t = 1.0 - std::pow(1.0 - u, 1.8);
        const double y = fondoY - t * (fondoY - horizonte.y());
        const double ancho = anchoCerca * 4.25 * (1.0 - t) + anchoLejos * 4.25 * t;
        painter.drawLine(QPointF{width() / 2.0 - ancho, y}, QPointF{width() / 2.0 + ancho, y});
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor{60, 255, 235, 34});
    painter.drawEllipse(jugador, 112.0, 42.0);
    painter.setBrush(QColor{20, 255, 230, 78});
    painter.drawEllipse(jugador, 68.0, 25.0);
    painter.setBrush(QColor{230, 255, 255, 54});
    painter.drawEllipse(jugador, 34.0, 13.0);
    if (!dibujarSpriteCentrado(painter, SpriteId::BaseSombra, jugador, QSizeF{136.0, 52.0})) {
        painter.setBrush(QColor{20, 36, 58, 180});
        painter.drawEllipse(jugador, 58.0, 22.0);
    }

    std::vector<const DiscoEnemigo*> discosOrdenados;
    for (const DiscoEnemigo* disco : nivel->obtenerDiscosEnemigos()) {
        if (disco != nullptr && !disco->estaDestruido()) {
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
        if (disco == nullptr || disco->estaDestruido()) {
            continue;
        }

        const QPointF centro = convertirDefensaAPantalla(disco->posicion());
        const double escala = escalaDefensa(disco->posicion());
        const double radio = 10.5 * escala;
        const int alphaLinea = static_cast<int>(std::clamp(95.0 + escala * 72.0, 95.0, 220.0));
        painter.setPen(QPen(QColor{255, 120, 95, alphaLinea}, 1.0 + escala * 1.2));
        painter.drawLine(centro, jugador);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor{255, 55, 55, static_cast<int>(35 + escala * 22.0)});
        painter.drawEllipse(centro, radio * 2.1, radio * 1.65);
        if (!dibujarSpriteCentrado(painter, SpriteId::DiscoEnemigo, centro, QSizeF{radio * 4.7, radio * 3.75})) {
            painter.setBrush(QColor{255, 80, 90});
            painter.drawEllipse(centro, radio, radio);
            painter.setBrush(QColor{255, 70, 80, 90});
            painter.drawEllipse(centro, radio * 1.65, radio * 1.65);
            painter.setBrush(QColor{255, 230, 170});
            painter.drawEllipse(centro, radio * 0.35, radio * 0.35);
        }
    }

    if (nivel->proyectilDefensorActivo()) {
        const QPointF proyectil = convertirDefensaAPantalla(nivel->posicionProyectilDefensor());
        const double escala = escalaDefensa(nivel->posicionProyectilDefensor());
        painter.setPen(QPen(QColor{80, 255, 235, 205}, 4));
        painter.drawLine(jugador, proyectil);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor{80, 255, 235, 70});
        painter.drawEllipse(proyectil, 12.0 * escala, 10.0 * escala);
        if (!dibujarSpriteCentrado(painter, SpriteId::ProyectilDefensor, proyectil, QSizeF{32.0 * escala, 24.0 * escala})) {
            painter.setBrush(QColor{110, 255, 245});
            painter.drawEllipse(proyectil, 6.0 * escala, 6.0 * escala);
            painter.setBrush(QColor{110, 255, 245, 95});
            painter.drawEllipse(proyectil, 13.0 * escala, 13.0 * escala);
        }
    }

    if (!dibujarSpriteCentrado(painter, SpriteId::JugadorDefensa, jugador, QSizeF{88.0, 88.0})) {
        painter.setBrush(QColor{40, 255, 210});
        painter.drawEllipse(jugador, 28.0, 28.0);
        painter.setBrush(QColor{230, 255, 245});
        painter.drawEllipse(jugador, 8.0, 8.0);
    }

    if (pulsoImpacto_.count() > 0) {
        const double progreso = progresoPulso(pulsoImpacto_, duracionPulsoImpacto);
        const QPointF centro = convertirDefensaAPantalla(posicionPulsoImpacto_);
        if (!dibujarSpriteCentrado(painter, SpriteId::ExplosionRoja, centro, QSizeF{62.0 + progreso * 40.0, 46.0 + progreso * 30.0})) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(QColor{255, 110, 95, static_cast<int>(230 * (1.0 - progreso))}, 3));
            painter.drawEllipse(centro, 10.0 + progreso * 36.0, 10.0 + progreso * 36.0);
            painter.setPen(Qt::NoPen);
        }
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
    painter.drawText(34, 154, defensa
        ? "Clic: accion | R: reiniciar | Esc/M: menu"
        : "Arrastra: trazar disco | R: reiniciar | Esc/M: menu");
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

    dibujarSpriteCentrado(
        painter,
        victoria ? SpriteId::Victoria : SpriteId::Derrota,
        QPointF{tarjeta.x() + 74.0, tarjeta.y() + 72.0},
        QSizeF{74.0, 74.0});

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
