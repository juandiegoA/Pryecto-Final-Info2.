#include "gui/MainWindow.h"

#include <QLabel>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Ultimate en TRON");
    setCentralWidget(new QLabel("Ultimate en TRON - base del proyecto", this));
    resize(960, 540);
}

