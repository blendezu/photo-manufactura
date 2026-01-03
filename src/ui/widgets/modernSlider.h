#pragma once

#include <QDoubleSpinBox>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QSlider>
#include <QWidget>

/**
 * @brief Modern slider widget with a sleek, professional appearance.
 *
 * Features:
 * - Smooth gradient track
 * - Custom handle with glow effect
 * - Animated value label
 * - Double-click to reset
 * - Click on slider name to show detail in InfoPanel
 */
class ModernSlider : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal handleGlow READ handleGlow WRITE setHandleGlow)

   public:
    explicit ModernSlider(const QString& label, int min, int max, int defaultValue = 0,
                          QWidget* parent = nullptr);

    int value() const;
    void setValue(int value);
    void setRange(int min, int max);
    void reset();
    void setTooltip(const QString& tooltip);
    void setUnit(const QString& unit);
    QString label() const {
        return m_labelText;
    }

    qreal handleGlow() const {
        return m_handleGlow;
    }
    void setHandleGlow(qreal glow);

   Q_SIGNALS:
    void valueChanged(int value);
    void sliderPressed();
    void sliderReleased();
    void detailRequested(const QString& sliderName, int min, int max, int value);

   private Q_SLOTS:
    void onSliderChanged(int value);
    void onSpinBoxChanged(int value);
    void onSliderPressed();
    void onSliderReleased();

   protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

   private:
    void setupUI();
    void updateSliderStyle();
    void animateHandleGlow(bool highlight);

    QString m_labelText;
    QString m_unit;
    QLabel* m_label;
    QLabel* m_valueLabel;
    QSlider* m_slider;
    QSpinBox* m_spinBox;
    int m_defaultValue;
    int m_min;
    int m_max;
    qreal m_handleGlow;
    QPropertyAnimation* m_glowAnimation;
    bool m_isHovered;
};

/**
 * @brief A compact inline slider for quick adjustments.
 * Used in toolbars or compact panels.
 */
class CompactSlider : public QWidget {
    Q_OBJECT

   public:
    explicit CompactSlider(const QString& label, int min, int max, int defaultValue = 0,
                           QWidget* parent = nullptr);

    int value() const;
    void setValue(int value);
    void reset();

   Q_SIGNALS:
    void valueChanged(int value);

   private:
    QLabel* m_label;
    QSlider* m_slider;
    QLabel* m_valueLabel;
    int m_defaultValue;
};
