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
    setWindowTitle(tr("Photo Manufactura !"));
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
    // Connect canvas mouse coordinates to info panel for debugging
    connect(m_canvasWidget, &CanvasWidget::mouseCoordinatesChanged, m_infoPanel,
            &InfoPanel::updateMouseCoords);
}

void MainWindow::onImageLoaded(const QImage& image, const QString& filePath) {
    // Display image on canvas
    m_canvasWidget->setImage(image);
    // Request fit-to-window from controller via signal
    emit m_canvasWidget->fitToWindowRequested();

    // Update info panel with image metadata
    QFileInfo fileInfo(filePath);
    m_infoPanel->updateImageInfo(fileInfo.fileName(), image.width(), image.height(),
                                 fileInfo.suffix().toUpper());
    m_infoPanel->updateFileSize(fileInfo.size());

    // Update window title
    setWindowTitle(tr("Photo Manufactura - %1").arg(fileInfo.fileName()));
}

void MainWindow::onFileSaved(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    setWindowTitle(tr("Photo Manufactura - %1").arg(fileInfo.fileName()));
}

void MainWindow::onError(const QString& message) {
    QMessageBox::warning(this, tr("Error"), message);
}
