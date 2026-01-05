#pragma once

#include <QObject>
#include <QPointF>
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
struct FourPointQuad;          // Forward declaration for perspective crop

/**
 * @brief Style transfer types for AI-based image styling
 */
enum class StyleTransferType {
    Mosaic = 0,
    Candy = 1,
    RainPrincess = 2,
    Udnie = 3,
    Pointillism = 4
};

/**
 * @brief Supported file formats for image operations
 */
namespace FileFormats {
// Supported image formats for opening (includes RAW formats)
inline constexpr const char* OpenFilter =
    "Image Files (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp);;"
    "RAW Files (*.raw *.cr2 *.cr3 *.nef *.arw *.dng *.orf *.rw2);;"
    "All Files (*)";

// Supported image formats for saving
inline constexpr const char* SaveFilter =
    "PNG Image (*.png);;"
    "JPEG Image (*.jpg *.jpeg);;"
    "BMP Image (*.bmp);;"
    "TIFF Image (*.tiff *.tif)";
}  // namespace FileFormats

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
    void newDocument();
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
    void adjustDenoise(int value);  // Detail adjustment
    void adjustClarity(int value);
    void adjustSharpening(int value);
    void adjustRotation(int degrees);  // Non-destructive rotation via ImageState
    void rotateImage(int degrees);     // Destructive rotation (90° steps)
    void applyStraighten(float angle, const QRect& cropRect);  // Destructive straighten
    void flipImage(int direction);                             // 0 = vertical, 1 = horizontal
    void cropImage(const QRect& cropArea);
    void perspectiveCropImage(const FourPointQuad& quad);  // Four-point perspective crop
    void requestPerspectiveCropMode();                     // Activate perspective crop in view
    void requestCropMode();                                // Activate standard crop mode
    void resizeImage(int width, int height);               // Resize image
    QSize currentImageSize() const;                        // Get current image dimensions
    void resetAdjustments();

    // Filter/Effect operations
    void applyFilterOriginal();
    void applyFilterGrayscale();
    void applyFilterVintage();
    void applyAutoEnhance();
    void applyStyleTransfer(StyleTransferType styleType);
    void setStyleTransferStrength(int strength);  // 0-100 range

    // Apply corrections permanently (bake adjustments into image)
    void applyCorrections();

    // Preset operations
    void applyPreset(const QString& presetName);
    void saveCurrentAsPreset(const QString& presetName);
    void showSavePresetDialog();

    // View operations
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

    // Theme operations
    void setTheme(const QString& themeName);
    void toggleTheme();
    void toggleHistogram();
    void toggleToolPanel();
    void toggleAdjustmentPanel();

    // Processing mode
    void setGpuMode(bool enabled);  // Toggle CPU/GPU processing

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
    void presetsChanged(const QStringList& userPresets);
    void adjustmentsChanged(int brightness, int contrast, int saturation, int exposure,
                            int highlights, int shadows, int whites, int blacks, int temperature,
                            int tint, int denoise, int clarity, int sharpening);
    void enablePerspectiveCropMode();   // Signal to view (Canvas)
    void enableCropMode(bool enabled);  // Toggle crop mode

   private slots:
    void onImageProcessingComplete();
    void onFileOperationComplete();

   private:
    void setupCommands();
    void connectUISignals();
    void connectModelSignals();
    void initializeServices();

    // Helper method to apply filter and update UI
    void applyFilterAndNotify(const QString& filterName, bool isRemoveFilter = false);

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