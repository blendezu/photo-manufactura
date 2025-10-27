#include "mainwindow.h"

#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

#include "bar/toolBar.h"
#include "widgets/canvasWidget.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setupMenus();
}
MainWindow::~MainWindow() {
    // Destructor implementation (if needed)
}

void MainWindow::setupUi() {
    setWindowTitle(tr("Photo Manufactura"));
    resize(800, 600);
    // Central Widget
    CanvasWidget* canvas = new CanvasWidget(this);
    setCentralWidget(canvas);
}
void MainWindow::setupMenus() {
    ToolBar* toolBar = new ToolBar(this);
    addToolBar(toolBar);
}
