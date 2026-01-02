#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class LabeledSlider;
class CollapsibleWidget;
class ToolPaletteWidget;

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

    // Reset signal
    void resetAllRequested();

   private:
    void setupUI();
    CollapsibleWidget* createBasicSection();
    CollapsibleWidget* createColorSection();
    CollapsibleWidget* createDetailSection();
    CollapsibleWidget* createToolSection();

    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_mainLayout;

    // Basic adjustments
    LabeledSlider* m_brightnessSlider;
    LabeledSlider* m_contrastSlider;
    LabeledSlider* m_exposureSlider;
    LabeledSlider* m_highlightsSlider;
    LabeledSlider* m_shadowsSlider;
    LabeledSlider* m_whitesSlider;
    LabeledSlider* m_blacksSlider;

    // Color adjustments
    LabeledSlider* m_temperatureSlider;
    LabeledSlider* m_tintSlider;
    LabeledSlider* m_saturationSlider;

    // Tool options
    ToolPaletteWidget* m_cropToolPalette;
    ToolPaletteWidget* m_rotateToolPalette;
    ToolPaletteWidget* m_flipToolPalette;
};