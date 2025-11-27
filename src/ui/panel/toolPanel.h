#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

class LabeledSlider;
class CollapsibleWidget;

class ToolPanel : public QWidget {
    Q_OBJECT
   public:
    explicit ToolPanel(QWidget* parent = nullptr);
    ~ToolPanel();

    void resetAllAdjustments();

   signals:
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

   private:
    void setupUI();
    CollapsibleWidget* createBasicSection();
    CollapsibleWidget* createColorSection();
    CollapsibleWidget* createDetailSection();

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
};