#include <QApplication>
#include <QDebug>

#include "controller/ApplicationController.h"
#include "controller/ApplicationWiring.h"
#include "ui/mainwindow.h"

int main(int argc, char* argv[]) {
    // Initialize Qt resources explicitly
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);

    // Create main window
    MainWindow window;

    // Create and initialize controller
    ApplicationController controller;
    controller.initialize();
    controller.setMainWindow(&window);

    // Wire all components together using the dedicated wiring class
    // This centralizes all signal/slot connections for easier maintenance
    ApplicationWiring wiring;
    wiring.wireComponents(&controller, &window);

    window.show();
    return app.exec();
}