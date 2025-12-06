#include <QApplication>
#include <QDebug>

#include "controller/ApplicationController.h"
#include "controller/ApplicationWiring.h"
#include "ui/mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qDebug() << "Starting Photo Manufactura Application";

    // Initialize Application Controller
    ApplicationController controller;
    controller.initialize();

    // Create and set up Main Window
    MainWindow mainWindow;
    controller.setMainWindow(&mainWindow);

    // Wire all UI components using ApplicationWiring
    ApplicationWiring wiring;
    wiring.wireComponents(&controller, &mainWindow);

    mainWindow.show();

    return app.exec();
}