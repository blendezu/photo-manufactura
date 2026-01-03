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

    // Geometry signals
    void rotateLeftRequested();
    void rotateRightRequested();
    void flipHorizontalRequested();
    void flipVerticalRequested();
    void cropRequested();
    void straightenRequested();

    // Crop option signals
    void cropAspectRatioChanged(int presetIndex);  // AspectRatioPreset as int
    void cropFixedSizeChanged(int width, int height);

    // Filter/Effect signals
    void filterOriginalRequested();
    void filterGrayscaleRequested();
    void filterVintageRequested();
    void filterAutoEnhanceRequested();

    // AI Style Transfer signals
    void styleTransferRequested(
        int styleType);  // 0=Mosaic, 1=Candy, 2=RainPrincess, 3=Udnie, 4=Pointillism

    // Reset signal
    void resetAllRequested();

   private:
    void setupUI();
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

    // Crop options
    QComboBox* m_cropModeCombo;
    QWidget* m_fixedSizeWidget;
    QSpinBox* m_cropWidthSpin;
    QSpinBox* m_cropHeightSpin;

    // Quick actions (modern buttons)
    ModernToolButton* m_autoEnhanceBtn;
    ModernToolButton* m_cropBtn;
    ModernToolButton* m_rotateBtn;
    ModernToolButton* m_filtersBtn;

    // Effects/Filters
    FilterGalleryWidget* m_filterGallery;
    FilterGalleryWidget* m_styleGallery;
};