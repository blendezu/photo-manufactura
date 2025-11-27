#include "mainwindow.h"

#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

#include "bar/subMenuEdit.h"
#include "bar/subMenuFile.h"
#include "bar/subMenuView.h"
#include "panel/infoPanel.h"
#include "panel/toolPanel.h"
#include "widgets/canvasWidget.h"
#include "widgets/themeManager.h"

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
