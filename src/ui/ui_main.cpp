#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    // Initialize Qt resources explicitly
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
