#include <QApplication>
#include <QDebug>

#include "controller/ApplicationController.h"
#include "ui/mainWindow.h"
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qDebug() << "Starting Photo Manufactura Application";

    // Initialize Application Controller
    ApplicationController controller;
    controller.initialize();

    // Create and set up Main Window
    MainWindow mainWindow;
    controller.setMainWindow(&mainWindow);
    mainWindow.show();

    return app.exec();
}