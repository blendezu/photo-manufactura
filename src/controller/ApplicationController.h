#pragma once

#include <QObject>
#include <QRect>
#include <QString>
#include <QVariantMap>
#include <memory>
#include <unordered_map>

// Forward declarations
class ICommand;
class MainWindow;
class CanvasWidget;
class ImageProcessingService;
class RawProcessingService;
class DocumentManager;

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

    // Initialization
    void initialize();
    void setMainWindow(MainWindow* mainWindow);

    // Command execution
    void executeCommand(const QString& commandName, const QVariantMap& parameters = {});
    void registerCommand(const QString& name, std::unique_ptr<ICommand> command);

    // State management
    void setState(const QString& key, const QVariant& value);
    QVariant getState(const QString& key) const;

    // Service access
    ImageProcessingService* getImageProcessingService() const;
    RawProcessingService* getRawProcessingService() const;
    DocumentManager* getDocumentManager() const;

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
    void rotateImage(int degrees);
    void cropImage(const QRect& cropArea);

    // View operations
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

   signals:
    // State change notifications
    void stateChanged(const QString& key, const QVariant& value);
    void fileOpened(const QString& filePath);
    void fileSaved(const QString& filePath);
    void imageProcessed();
    void errorOccurred(const QString& message);

   private slots:
    void onImageProcessingComplete();
    void onFileOperationComplete();

   private:
    void setupCommands();
    void connectUISignals();
    void initializeServices();

    // UI components
    MainWindow* m_mainWindow;
    CanvasWidget* m_canvas;

    // Services (business logic)
    std::unique_ptr<ImageProcessingService> m_imageProcessingService;
    std::unique_ptr<RawProcessingService> m_rawProcessingService;
    std::unique_ptr<DocumentManager> m_documentManager;

    // Command management
    std::unordered_map<QString, std::unique_ptr<ICommand>> m_commands;

    // Application state
    QVariantMap m_applicationState;
};