#pragma once

#include <QObject>
#include <QRect>
#include <QString>
#include <QVariantMap>
#include <memory>
#include <unordered_map>

// Model includes
#include "../model/AppState.h"
#include "../model/DocumentManager.h"

// Forward declarations
class ICommand;
class QWidget;
class ImageProcessingService;  // Service layer for image processing

/**
 * @brief Main Application Controller
 *
 * Orchestrates all business operations and coordinates between UI and data layers.
 * Implements the Controller part of the MVC pattern.
 */
class ApplicationController : public QObject {
    Q_OBJECT

   public:
    explicit ApplicationController(QObject* parent = nullptr);
    ~ApplicationController();

    /** Initialize the application controller
     * Sets up commands, services, and initial state
     * @return void
     */
    void initialize();

    /** Set the main application window
     * Connects UI signals to controller slots
     * @param mainWindow Pointer to the main window widget
     */
    void setMainWindow(QWidget* mainWindow);

    /**
     * Execute a registered command by name
     * @param commandName Name of the command to execute
     * @param parameters Optional parameters for the command
     * @return void
     */
    void executeCommand(const QString& commandName, const QVariantMap& parameters = {});
    /**
     * Register a command with the controller
     * @param name Name of the command
     * @param command Unique pointer to the command implementation
     * @return void
     */
    void registerCommand(const QString& name, std::unique_ptr<ICommand> command);

    /** Set application state value
     * @param key State key
     * @param value State value
     */
    void setState(const QString& key, const QVariant& value);
    /** Get application state value
     * @param key State key
     * @return State value
     */
    QVariant getState(const QString& key) const;

    // Service access - to be implemented when services exist
    // ImageProcessingService* getImageProcessingService() const;
    // RawProcessingService* getRawProcessingService() const;
    DocumentManager* getDocumentManager() const {
        return m_documentManager.get();
    }
    AppState* getAppState() const {
        return m_appState.get();
    }
    AdjustmentSettings* getAdjustments() const;

   public slots:
    // File operations
    void openFile();
    void saveFile();
    void saveAsFile();
    void closeFile();
    void exitApplication();

    // Edit operations
    void undo();
    void redo();
    void copy();
    void paste();
    void cut();

    // Image operations
    void applyFilter(const QString& filterName);
    void adjustBrightness(int value);
    void adjustContrast(int value);
    void adjustExposure(int value);
    void adjustHighlights(int value);
    void adjustShadows(int value);
    void adjustWhites(int value);
    void adjustBlacks(int value);
    void adjustTemperature(int value);
    void adjustTint(int value);
    void adjustSaturation(int value);
    void rotateImage(int degrees);
    void cropImage(const QRect& cropArea);
    void resetAdjustments();

    // View operations
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

    // Theme operations
    void setTheme(const QString& themeName);
    void toggleHistogram();
    void toggleToolPanel();
    void toggleAdjustmentPanel();

   signals:
    // State change notifications
    void stateChanged(const QString& key, const QVariant& value);
    void imageLoaded(const QImage& image, const QString& filePath);
    void fileOpened(const QString& filePath);
    void fileSaved(const QString& filePath);
    void fileClosed();
    void imageProcessed();
    void errorOccurred(const QString& message);
    void themeChanged(const QString& theme);
    void zoomChanged(double level);

   private slots:
    void onImageProcessingComplete();
    void onFileOperationComplete();

   private:
    void setupCommands();
    void connectUISignals();
    void connectModelSignals();
    void initializeServices();

    // UI components (stored as QWidget* for loose coupling)
    QWidget* m_mainWindow;

    // Model layer
    std::unique_ptr<DocumentManager> m_documentManager;
    std::unique_ptr<AppState> m_appState;

    // Services layer
    // Uncomment when image_processing component is linked:
    // std::unique_ptr<ImageProcessingService> m_imageProcessingService;

    // Command management
    std::unordered_map<QString, std::unique_ptr<ICommand>> m_commands;

    // Application state
    QVariantMap m_applicationState;
};