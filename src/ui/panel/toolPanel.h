#pragma once

#include <QComboBox>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

class ModernSlider;
class ModernCollapsible;
class ModernToolButton;
class ModernButton;
class IconButton;
class FilterGalleryWidget;

// Forward declare crop types from canvasWidget
enum class CropType;
enum class AspectRatioPreset;

// Forward declare style transfer type from ApplicationController
enum class StyleTransferType;

class ToolPanel : public QWidget {
    Q_OBJECT
   public:
    explicit ToolPanel(QWidget* parent = nullptr);
    ~ToolPanel();

    void resetAllAdjustments();

   Q_SIGNALS:
    // Adjustment signals
    void brightnessChanged(int value);
    void contrastChanged(int value);
    void saturationChanged(int value);
    void exposureChanged(int value);
    void highlightsChanged(int value);
    void shadowsChanged(int value);
    void whitesChanged(int value);
    void blacksChanged(int value);
    void temperatureChanged(int value);
    void tintChanged(int value);

    // Generic signal for history interaction
    void adjustmentStarted();                                 // Emitted on slider press
    void adjustmentFinished(const QString& name, int value);  // Emitted on slider release

    // Detail signals
    void denoiseChanged(int value);
    void clarityChanged(int value);
    void sharpeningChanged(int value);

    // Geometry signals
    void rotateLeftRequested();
    void rotateRightRequested();
    void flipHorizontalRequested();
    void flipVerticalRequested();
    void cropRequested();
    void straightenRequested();

    void rotateAngleChanged(int degrees);         // Custom angle rotation
    void resizeConfirmed(int width, int height);  // Resize with specific dimensions
    void perspectiveCropRequested();              // Request perspective crop mode

    // Straighten mode signals
    void straightenModeToggled(bool enabled);            // Straighten mode toggle
    void applyStraightenRequested();                     // Apply straighten with auto-crop
    void straightenAspectRatioChanged(int aspectIndex);  // Straighten aspect ratio changed

    // Crop option signals
    void cropAspectRatioChanged(int presetIndex);  // AspectRatioPreset as int
    void cropFixedSizeChanged(int width, int height);

    // Filter/Effect signals
    void filterOriginalRequested();
    void filterGrayscaleRequested();
    void filterVintageRequested();
    void autoLightRequested();

    // AI Style Transfer signals
    void styleTransferRequested(StyleTransferType styleType);
    void styleStrengthChanged(int strength);

    // Preset signals
    void presetSelected(const QString& presetName);
    void savePresetRequested(const QString& presetName);
    void resetToOriginalRequested();  // Global reset (file reload)

    // Reset signal
    void resetAllRequested();

    // Apply corrections permanently
    void applyRequested();

    // Compare mode signal (checkable button)
    void compareModeToggled(bool enabled);

    // Zoom mode signal
    void zoomModeToggled(bool enabled);

    // Save preset button clicked (shows dialog)
    void savePresetButtonClicked();

   public slots:
    void setZoomModeChecked(bool checked);
    void refreshPresets(const QStringList& userPresets);  // Refresh combo with user presets
    void updateSliders(int brightness, int contrast, int saturation, int exposure, int highlights,
                       int shadows, int whites, int blacks, int temperature, int tint, int denoise,
                       int clarity, int sharpening);  // Update sliders from preset
    void setColorControlsEnabled(
        bool enabled);  // Enable/disable color sliders based on active effect
    void updateImageInfo(const QImage& image);  // Receive current image info
    void showResizeDialog();

   private:
    void setupUI();
    ModernCollapsible* createPresetsSection();
    ModernCollapsible* createBasicSection();
    ModernCollapsible* createColorSection();
    ModernCollapsible* createDetailSection();
    ModernCollapsible* createToolSection();
    ModernCollapsible* createEffectsSection();
    ModernCollapsible* createAIStyleSection();
    QWidget* createQuickActionsBar();

    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_mainLayout;

    // Basic adjustments (using modern sliders)
    ModernSlider* m_brightnessSlider;
    ModernSlider* m_contrastSlider;
    ModernSlider* m_exposureSlider;
    ModernSlider* m_highlightsSlider;
    ModernSlider* m_shadowsSlider;
    ModernSlider* m_whitesSlider;
    ModernSlider* m_blacksSlider;

    // Color adjustments
    ModernSlider* m_temperatureSlider;
    ModernSlider* m_tintSlider;
    ModernSlider* m_saturationSlider;

    // Detail adjustments
    ModernSlider* m_denoiseSlider;
    ModernSlider* m_claritySlider;
    ModernSlider* m_sharpeningSlider;

    // Crop options
    QComboBox* m_cropModeCombo;
    QWidget* m_fixedSizeWidget;
    QSpinBox* m_cropWidthSpin;
    QSpinBox* m_cropHeightSpin;

    // Rotation slider (for real-time rotation)
    ModernSlider* m_rotationSlider;

    // Straighten mode controls
    ModernToolButton* m_straightenToggle;
    ModernButton* m_applyStraightenBtn;
    QComboBox* m_straightenRatioCombo;

    // Quick actions (modern buttons)
    ModernToolButton* m_autoEnhanceBtn;
    ModernToolButton* m_compareBtn;
    ModernToolButton* m_zoomBtn;

    // Effects/Filters
    FilterGalleryWidget* m_filterGallery;
    FilterGalleryWidget* m_styleGallery;
    ModernSlider* m_styleStrengthSlider;

    // Collapsible sections (for enabling/disabling)
    ModernCollapsible* m_colorSection;

    // State
    QSize m_currentImageSize;

    // Presets
    QComboBox* m_presetCombo;
};