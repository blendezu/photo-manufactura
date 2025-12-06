#pragma once

#include <QObject>
#include <memory>

// Forward declarations
class ApplicationController;
class MainWindow;
class SubMenuFile;
class SubMenuEdit;
class SubMenuView;
class CanvasWidget;
class ToolPanel;
class InfoPanel;

/**
 * @brief Handles all signal/slot connections between components
 *
 * This class centralizes all the wiring between UI components and the controller,
 * making the main() function cleaner and the connections easier to maintain.
 * Implements a form of Dependency Injection by managing component relationships.
 */
class ApplicationWiring : public QObject {
    Q_OBJECT

   public:
    explicit ApplicationWiring(QObject* parent = nullptr);
    ~ApplicationWiring();

    /**
     * @brief Wire all components together
     * @param controller The application controller
     * @param mainWindow The main window containing UI components
     */
    void wireComponents(ApplicationController* controller, MainWindow* mainWindow);

    /**
     * @brief Disconnect all wired connections
     * Call this before destroying components
     */
    void unwireComponents();

   private:
    /**
     * @brief Connect file menu signals to controller slots
     */
    void wireFileMenu(SubMenuFile* fileMenu, ApplicationController* controller);

    /**
     * @brief Connect edit menu signals to controller slots
     */
    void wireEditMenu(SubMenuEdit* editMenu, ApplicationController* controller);

    /**
     * @brief Connect view menu signals to controller slots
     */
    void wireViewMenu(SubMenuView* viewMenu, ApplicationController* controller);

    /**
     * @brief Connect canvas widget signals to controller slots
     */
    void wireCanvas(CanvasWidget* canvas, ApplicationController* controller);

    /**
     * @brief Connect tool panel signals to controller slots
     */
    void wireToolPanel(ToolPanel* toolPanel, ApplicationController* controller);

    /**
     * @brief Connect info panel signals to controller slots
     */
    void wireInfoPanel(InfoPanel* infoPanel, ApplicationController* controller);

    /**
     * @brief Connect controller signals to main window slots
     */
    void wireControllerToMainWindow(ApplicationController* controller, MainWindow* mainWindow);

    /**
     * @brief Connect controller signals to canvas
     */
    void wireControllerToCanvas(ApplicationController* controller, CanvasWidget* canvas);

    /**
     * @brief Connect document manager to canvas and info panel
     * Handles document open, close, and processed image updates
     */
    void wireDocumentToCanvas(ApplicationController* controller, MainWindow* mainWindow);

    // Store connections for cleanup
    QList<QMetaObject::Connection> m_connections;

    // Cached component pointers for unwiring
    ApplicationController* m_controller = nullptr;
    MainWindow* m_mainWindow = nullptr;
};
