#include "mainwindow.h"

#include <QAction>
#include <QDockWidget>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

#include "bar/subMenuEdit.h"
#include "bar/subMenuFile.h"
#include "bar/subMenuView.h"
#include "canvas/canvasWidget.h"
#include "panel/infoPanel.h"
#include "panel/toolPanel.h"
#include "resources/theme/themeManager.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_canvasWidget(nullptr),
      m_toolPanel(nullptr),
      m_infoPanel(nullptr),
      m_fileMenu(nullptr),
      m_editMenu(nullptr),
      m_viewMenu(nullptr) {
    setupUi();
    setupMenuBar();
    setupDockPanels();
    setupConnections();
}

MainWindow::~MainWindow() {
    // Qt handles child widget deletion automatically
}

void MainWindow::setupUi() {
    setWindowTitle(tr("Photo Manufactura"));
    resize(1200, 800);

    // Apply theme using ThemeManager
    ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);

    // Central Widget - Main canvas for image display
    m_canvasWidget = new CanvasWidget(this);
    setCentralWidget(m_canvasWidget);
}

void MainWindow::setupMenuBar() {
    // Create File menu
    m_fileMenu = new SubMenuFile(this);
    menuBar()->addMenu(m_fileMenu);

    // Create Edit menu
    m_editMenu = new SubMenuEdit(this);
    menuBar()->addMenu(m_editMenu);

    // Create View menu
    m_viewMenu = new SubMenuView(this);
    menuBar()->addMenu(m_viewMenu);

    // TODO: Add Image and other menus as needed
}

void MainWindow::setupDockPanels() {
    // Left Panel - Tool Panel for adjustments (brightness, contrast, etc.)
    QDockWidget* toolDock = new QDockWidget(tr("Adjustments"), this);
    toolDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolPanel = new ToolPanel(this);
    toolDock->setWidget(m_toolPanel);
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);

    // Right Panel - Info Panel for metadata, histogram, etc.
    QDockWidget* infoDock = new QDockWidget(tr("Info"), this);
    infoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_infoPanel = new InfoPanel(this);
    infoDock->setWidget(m_infoPanel);
    addDockWidget(Qt::RightDockWidgetArea, infoDock);
}

void MainWindow::setupConnections() {
    // Connect File Menu signals to MainWindow slots
    connect(m_fileMenu, &SubMenuFile::imageLoaded, this, &MainWindow::loadImage);
    connect(m_fileMenu, &SubMenuFile::imageSaveRequested, this, &MainWindow::saveImage);
}

void MainWindow::loadImage(const QString& filePath) {
    // TODO: MOVE TO CONTROLLER: Image loading logic should be handled by ApplicationController
    // The controller should handle:
    //   1. Loading the image via DocumentManager or ImageProcessingService
    //   2. Validating the image (null check, format support)
    //   3. Managing m_currentFilePath state via setState()
    //   4. Extracting and storing metadata
    // The MainWindow should receive a signal from controller with the loaded QImage
    // and only handle UI updates (setImage, zoomToFit, updateWindowTitle)

    QImage image(filePath);  // TODO: MOVE TO CONTROLLER: Use DocumentManager to load image

    if (image.isNull()) {  // TODO: MOVE TO CONTROLLER: Validation should be in controller
        QMessageBox::warning(this, tr("Error"), tr("Failed to load image: %1").arg(filePath));
        return;
    }

    // Store current file path
    m_currentFilePath = filePath;  // TODO: MOVE TO CONTROLLER: Use
                                   // ApplicationController::setState("currentFile", filePath)

    // Display image on canvas - This stays in UI
    m_canvasWidget->setImage(image);
    m_canvasWidget->zoomToFit();

    // Update info panel with image metadata - This stays in UI, but metadata extraction moves to
    // controller
    QFileInfo fileInfo(filePath);  // TODO: MOVE TO CONTROLLER: Metadata extraction
    m_infoPanel->updateImageInfo(fileInfo.fileName(), image.width(), image.height(),
                                 fileInfo.suffix().toUpper());

    // Update window title - This stays in UI
    setWindowTitle(tr("Photo Manufactura - %1").arg(fileInfo.fileName()));
}

void MainWindow::saveImage(const QString& filePath) {
    // TODO: MOVE TO CONTROLLER: Image saving logic should be handled by
    // ApplicationController::saveFile() The controller should handle:
    //   1. Getting the processed image from ImagePipeline
    //   2. Coordinating with DocumentManager to save the file
    //   3. Handling save errors and updating state
    //   4. Emitting fileSaved signal on success
    // The MainWindow should only receive success/error signals from controller

    // TODO: Implement save using processed image from pipeline
    // For now, show placeholder message
    QMessageBox::information(
        this, tr("Save"),
        tr("Save functionality will be connected to ImagePipeline.\nPath: %1").arg(filePath));
}
