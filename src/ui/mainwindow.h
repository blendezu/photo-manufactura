#pragma once

#include <QMainWindow>

class SubMenuFile;
class SubMenuEdit;
class SubMenuView;
class CanvasWidget;
class ToolPanel;
class InfoPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Public accessors for controller
    CanvasWidget* getCanvasWidget() const {
        return m_canvasWidget;
    }

   public slots:
    void loadImage(const QString& filePath);
    void saveImage(const QString& filePath);

   private:
    void setupUi();
    void setupMenuBar();
    void setupDockPanels();
    void setupConnections();

   private:
    // UI Components
    CanvasWidget* m_canvasWidget;
    ToolPanel* m_toolPanel;
    InfoPanel* m_infoPanel;

    // Menu components
    SubMenuFile* m_fileMenu;
    SubMenuEdit* m_editMenu;
    SubMenuView* m_viewMenu;

    // TODO: MOVE TO CONTROLLER: m_currentFilePath should be managed by ApplicationController state
    // Use ApplicationController::getState("currentFile") instead
    // Current file
    QString m_currentFilePath;
};