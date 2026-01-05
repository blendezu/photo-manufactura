#include "modernSlider.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>

ModernSlider::ModernSlider(const QString& label, int min, int max, int defaultValue,
                           QWidget* parent)
    : QWidget(parent),
      m_labelText(label),
      m_defaultValue(defaultValue),
      m_min(min),
      m_max(max),
      m_handleGlow(0.0),
      m_isHovered(false) {
    setupUI();
    m_glowAnimation = new QPropertyAnimation(this, "handleGlow", this);
    m_glowAnimation->setDuration(150);
}

void ModernSlider::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 6, 4, 6);
    mainLayout->setSpacing(4);

    // Header row with label and value
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    m_label = new QLabel(m_labelText, this);
    m_label->setStyleSheet(R"(
        QLabel {
            color: #c0c0c0;
            font-size: 11px;
            font-weight: 500;
            letter-spacing: 0.3px;
        }
    )");
    m_label->setCursor(Qt::PointingHandCursor);

    m_valueLabel = new QLabel(QString::number(m_defaultValue), this);
    m_valueLabel->setStyleSheet(R"(
        QLabel {
            color: #888;
            font-size: 10px;
            font-family: 'SF Mono', 'Consolas', monospace;
        }
    )");
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    headerLayout->addWidget(m_label);
    headerLayout->addStretch();
    headerLayout->addWidget(m_valueLabel);

    // Slider row
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    sliderLayout->setSpacing(8);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimum(m_min);
    m_slider->setMaximum(m_max);
    m_slider->setValue(m_defaultValue);
    m_slider->setMinimumHeight(20);
    m_slider->setCursor(Qt::PointingHandCursor);

    // SpinBox for precise input
    m_spinBox = new QSpinBox(this);
    m_spinBox->setMinimum(m_min);
    m_spinBox->setMaximum(m_max);
    m_spinBox->setValue(m_defaultValue);
    m_spinBox->setFixedWidth(48);
    m_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinBox->setAlignment(Qt::AlignCenter);
    m_spinBox->setStyleSheet(R"(
        QSpinBox {
            background: #252525;
            border: 1px solid #3a3a3a;
            border-radius: 4px;
            padding: 2px 4px;
            color: #d0d0d0;
            font-size: 11px;
            font-family: 'SF Mono', 'Consolas', monospace;
        }
        QSpinBox:focus {
            border: 1px solid #0078d4;
            background: #2a2a2a;
        }
        QSpinBox:hover {
            border: 1px solid #4a4a4a;
        }
    )");

    sliderLayout->addWidget(m_slider);
    sliderLayout->addWidget(m_spinBox);

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(sliderLayout);

    // Apply initial slider style
    updateSliderStyle();

    // Connect signals
    connect(m_slider, &QSlider::valueChanged, this, &ModernSlider::onSliderChanged);
    connect(m_slider, &QSlider::sliderPressed, this, &ModernSlider::onSliderPressed);
    connect(m_slider, &QSlider::sliderReleased, this, &ModernSlider::onSliderReleased);
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &ModernSlider::onSpinBoxChanged);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ModernSlider::updateSliderStyle() {
    int glowIntensity = static_cast<int>(m_handleGlow * 30);
    QString accentColor = m_isHovered ? "#0098ff" : "#0078d4";

    QString style = QString(R"(
        QSlider::groove:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #303030, stop:0.5 #383838, stop:1 #303030);
            height: 4px;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0058a4, stop:1 %1);
            height: 4px;
            border-radius: 2px;
        }
        QSlider::add-page:horizontal {
            background: #303030;
            height: 4px;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f0f0f0, stop:0.4 #e0e0e0, stop:1 #c8c8c8);
            border: 1px solid #888;
            width: 14px;
            height: 14px;
            margin: -6px 0;
            border-radius: 7px;
        }
        QSlider::handle:horizontal:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ffffff, stop:0.4 #f0f0f0, stop:1 #e0e0e0);
            border: 1px solid %1;
        }
        QSlider::handle:horizontal:pressed {
            background: %1;
            border: 2px solid #ffffff;
        }
    )")
                        .arg(accentColor);

    m_slider->setStyleSheet(style);
}

void ModernSlider::setHandleGlow(qreal glow) {
    m_handleGlow = glow;
    updateSliderStyle();
}

void ModernSlider::animateHandleGlow(bool highlight) {
    m_glowAnimation->stop();
    m_glowAnimation->setStartValue(m_handleGlow);
    m_glowAnimation->setEndValue(highlight ? 1.0 : 0.0);
    m_glowAnimation->start();
}

int ModernSlider::value() const {
    return m_slider->value();
}

void ModernSlider::setValue(int value) {
    m_slider->blockSignals(true);
    m_spinBox->blockSignals(true);
    m_slider->setValue(value);
    m_spinBox->setValue(value);
    updateDisplayValue(value);
    m_slider->blockSignals(false);
    m_spinBox->blockSignals(false);
}

void ModernSlider::setRange(int min, int max) {
    m_min = min;
    m_max = max;
    m_slider->setRange(min, max);
    m_spinBox->setRange(min, max);
}

void ModernSlider::reset() {
    setValue(m_defaultValue);
    Q_EMIT valueChanged(m_defaultValue);
}

void ModernSlider::setTooltip(const QString& tooltip) {
    m_label->setToolTip(tooltip);
    m_slider->setToolTip(tooltip + "\n\nDouble-click to reset");
    m_spinBox->setToolTip(tooltip);
}

void ModernSlider::setUnit(const QString& unit) {
    m_unit = unit;
    updateDisplayValue(m_slider->value());
}

void ModernSlider::setDisplayDivisor(float divisor) {
    m_displayDivisor = (divisor > 0.001f) ? divisor : 1.0f;
    updateDisplayValue(m_slider->value());
}

void ModernSlider::updateDisplayValue(int rawValue) {
    float displayValue = rawValue / m_displayDivisor;
    QString text;
    if (m_displayDivisor > 1.0f && displayValue == static_cast<int>(displayValue)) {
        // Show as integer if divisor result is whole number
        text = (displayValue > 0 ? "+" : "") + QString::number(static_cast<int>(displayValue)) +
               m_unit;
    } else if (m_displayDivisor > 1.0f) {
        // Show one decimal for fractional EV
        text = (displayValue > 0 ? "+" : "") + QString::number(displayValue, 'f', 1) + m_unit;
    } else {
        text = QString::number(rawValue) + m_unit;
    }
    m_valueLabel->setText(text);
}

void ModernSlider::onSliderChanged(int value) {
    m_spinBox->blockSignals(true);
    m_spinBox->setValue(value);
    m_spinBox->blockSignals(false);
    updateDisplayValue(value);
    Q_EMIT valueChanged(value);
}

void ModernSlider::onSpinBoxChanged(int value) {
    m_slider->blockSignals(true);
    m_slider->setValue(value);
    m_slider->blockSignals(false);
    updateDisplayValue(value);
    Q_EMIT valueChanged(value);
}

void ModernSlider::onSliderPressed() {
    animateHandleGlow(true);
    Q_EMIT sliderPressed();
}

void ModernSlider::onSliderReleased() {
    animateHandleGlow(false);
    Q_EMIT sliderReleased();
}

void ModernSlider::mouseDoubleClickEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    reset();
    QWidget::mouseDoubleClickEvent(event);
}

void ModernSlider::enterEvent(QEnterEvent* event) {
    m_isHovered = true;
    updateSliderStyle();
    m_label->setStyleSheet(R"(
        QLabel {
            color: #ffffff;
            font-size: 11px;
            font-weight: 500;
            letter-spacing: 0.3px;
        }
    )");
    QWidget::enterEvent(event);
}

void ModernSlider::leaveEvent(QEvent* event) {
    m_isHovered = false;
    updateSliderStyle();
    m_label->setStyleSheet(R"(
        QLabel {
            color: #c0c0c0;
            font-size: 11px;
            font-weight: 500;
            letter-spacing: 0.3px;
        }
    )");
    QWidget::leaveEvent(event);
}

void ModernSlider::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // Hide spinbox when slider is too narrow (< 200px)
    // Show spinbox when wider (>= 220px) with hysteresis to prevent flickering
    const int hideThreshold = 200;
    const int showThreshold = 220;

    if (event->size().width() < hideThreshold && m_spinBox->isVisible()) {
        m_spinBox->hide();
    } else if (event->size().width() >= showThreshold && !m_spinBox->isVisible()) {
        m_spinBox->show();
    }
}

void ModernSlider::setShowTickMarks(bool show, int count) {
    m_showTickMarks = show;
    m_tickCount = count;
    update();
}

void ModernSlider::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    if (!m_showTickMarks || m_tickCount < 2)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Get slider geometry
    QRect sliderRect = m_slider->geometry();
    int handleWidth = 14;  // Match slider handle width
    int tickAreaLeft = sliderRect.left() + handleWidth / 2;
    int tickAreaRight = sliderRect.right() - handleWidth / 2;
    int tickAreaWidth = tickAreaRight - tickAreaLeft;

    // Draw tick marks below slider
    int tickY = sliderRect.bottom() + 4;
    int tickHeight = 6;

    painter.setPen(QPen(QColor(100, 100, 100), 1));
    QFont tickFont("SF Mono", 8);
    tickFont.setStyleHint(QFont::Monospace);
    painter.setFont(tickFont);

    for (int i = 0; i < m_tickCount; ++i) {
        float ratio = static_cast<float>(i) / (m_tickCount - 1);
        int x = tickAreaLeft + static_cast<int>(ratio * tickAreaWidth);

        // Taller tick at center (0)
        bool isCenter = (i == m_tickCount / 2);
        int h = isCenter ? tickHeight + 4 : tickHeight;

        // Draw tick line
        painter.setPen(QPen(isCenter ? QColor(150, 150, 150) : QColor(80, 80, 80), 1));
        painter.drawLine(x, tickY, x, tickY + h);

        // Draw number labels at key positions (-5, 0, +5)
        if (i == 0 || isCenter || i == m_tickCount - 1) {
            int evValue = m_min + (m_max - m_min) * i / (m_tickCount - 1);
            // Convert to EV (divide by 10 for -50 to 50 range)
            float ev = evValue / 10.0f;
            QString label = (ev > 0 ? "+" : "") + QString::number(static_cast<int>(ev));

            painter.setPen(QColor(120, 120, 120));
            QRect textRect(x - 15, tickY + h + 2, 30, 12);
            painter.drawText(textRect, Qt::AlignCenter, label);
        }
    }
}

// ============================================================================
// CompactSlider Implementation
// ============================================================================

CompactSlider::CompactSlider(const QString& label, int min, int max, int defaultValue,
                             QWidget* parent)
    : QWidget(parent), m_defaultValue(defaultValue) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(6);

    m_label = new QLabel(label, this);
    m_label->setStyleSheet("color: #aaa; font-size: 10px;");
    m_label->setFixedWidth(60);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(min, max);
    m_slider->setValue(defaultValue);
    m_slider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            background: #333;
            height: 3px;
            border-radius: 1px;
        }
        QSlider::sub-page:horizontal {
            background: #0078d4;
            height: 3px;
            border-radius: 1px;
        }
        QSlider::handle:horizontal {
            background: #ddd;
            border: 1px solid #888;
            width: 10px;
            height: 10px;
            margin: -4px 0;
            border-radius: 5px;
        }
        QSlider::handle:horizontal:hover {
            background: #fff;
        }
    )");

    m_valueLabel = new QLabel(QString::number(defaultValue), this);
    m_valueLabel->setStyleSheet("color: #888; font-size: 10px; font-family: monospace;");
    m_valueLabel->setFixedWidth(30);
    m_valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(m_label);
    layout->addWidget(m_slider, 1);
    layout->addWidget(m_valueLabel);

    connect(m_slider, &QSlider::valueChanged, this, [this](int val) {
        m_valueLabel->setText(QString::number(val));
        Q_EMIT valueChanged(val);
    });

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(24);
}

int CompactSlider::value() const {
    return m_slider->value();
}

void CompactSlider::setValue(int value) {
    m_slider->blockSignals(true);
    m_slider->setValue(value);
    m_valueLabel->setText(QString::number(value));
    m_slider->blockSignals(false);
}

void CompactSlider::reset() {
    setValue(m_defaultValue);
    Q_EMIT valueChanged(m_defaultValue);
}
