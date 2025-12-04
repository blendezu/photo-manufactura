#include <QApplication>

#include "../controller/ApplicationController.h"
#include "bar/subMenuFile.h"
#include "mainwindow.h"

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

    // Connect File Menu signals to Controller
    QObject::connect(window.getFileMenu(), &SubMenuFile::newDocumentRequested, &controller,
                     &ApplicationController::closeFile);  // TODO: Add newDocument slot
    QObject::connect(window.getFileMenu(), &SubMenuFile::openFileRequested, &controller,
                     &ApplicationController::openFile);
    QObject::connect(window.getFileMenu(), &SubMenuFile::saveFileRequested, &controller,
                     &ApplicationController::saveFile);
    QObject::connect(window.getFileMenu(), &SubMenuFile::saveAsFileRequested, &controller,
                     &ApplicationController::saveAsFile);
    QObject::connect(window.getFileMenu(), &SubMenuFile::exitRequested, &controller,
                     &ApplicationController::exitApplication);

    // Connect Controller signals to MainWindow
    QObject::connect(&controller, &ApplicationController::imageLoaded, &window,
                     &MainWindow::onImageLoaded);
    QObject::connect(&controller, &ApplicationController::fileSaved, &window,
                     &MainWindow::onFileSaved);
    QObject::connect(&controller, &ApplicationController::errorOccurred, &window,
                     &MainWindow::onError);

    window.show();
    return app.exec();
}
