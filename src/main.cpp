// main.cpp : starts the ElectroMart application
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setPalette(app.style()->standardPalette());    // same light look on every computer

    MainWindow w;
    w.show();
    return app.exec();
}
