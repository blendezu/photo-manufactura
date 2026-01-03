#include "labeledSlider.h"

#include <QMouseEvent>
#include <QPushButton>

LabeledSlider::LabeledSlider(const QString& label, int min, int max, int defaultValue,
                             QWidget* parent)
    : QWidget(parent), m_defaultValue(defaultValue) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    // Label
    m_label = new QLabel(label, this);
    m_label->setStyleSheet("font-weight: 500; color: #bbb; font-size: 12px;");
    m_label->setWordWrap(false);
    m_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Horizontal layout for slider and spinbox
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(8);

    // Slider
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimum(min);
    m_slider->setMaximum(max);
    m_slider->setValue(defaultValue);
    m_slider->setTickPosition(QSlider::TicksBelow);
    m_slider->setTickInterval((max - min) / 4);
    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_slider->setMinimumWidth(120);

    // SpinBox
    m_spinBox = new QSpinBox(this);
    m_spinBox->setMinimum(min);
    m_spinBox->setMaximum(max);
    m_spinBox->setValue(defaultValue);
    m_spinBox->setFixedWidth(60);
    m_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinBox->setAlignment(Qt::AlignCenter);
    m_spinBox->setStyleSheet(
        "QSpinBox { "
        "  background: #2a2a2a; "
        "  border: 1px solid #444; "
        "  border-radius: 3px; "
        "  padding: 2px; "
        "  color: #ddd; "
        "} "
        "QSpinBox:focus { "
        "  border: 1px solid #0078d4; "
        "}");

    controlLayout->addWidget(m_slider);
    controlLayout->addWidget(m_spinBox);

    mainLayout->addWidget(m_label);
    mainLayout->addLayout(controlLayout);

    // Set size policy for responsive behavior
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Connect signals
    connect(m_slider, &QSlider::valueChanged, this, &LabeledSlider::onSliderChanged);
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &LabeledSlider::onSpinBoxChanged);
}

int LabeledSlider::value() const {
    return m_slider->value();
}

void LabeledSlider::setValue(int value) {
    m_slider->blockSignals(true);
    m_spinBox->blockSignals(true);
    m_slider->setValue(value);
    m_spinBox->setValue(value);
    m_slider->blockSignals(false);
    m_spinBox->blockSignals(false);
}

void LabeledSlider::setRange(int min, int max) {
    m_slider->setRange(min, max);
    m_spinBox->setRange(min, max);
}

void LabeledSlider::reset() {
    setValue(m_defaultValue);
}

void LabeledSlider::onSliderChanged(int value) {
    m_spinBox->blockSignals(true);
    m_spinBox->setValue(value);
    m_spinBox->blockSignals(false);
    emit valueChanged(value);
}

void LabeledSlider::onSpinBoxChanged(int value) {
    m_slider->blockSignals(true);
    m_slider->setValue(value);
    m_slider->blockSignals(false);
    emit valueChanged(value);
}

void LabeledSlider::setTooltip(const QString& tooltip) {
    m_label->setToolTip(tooltip);
    m_slider->setToolTip(tooltip + "\n\nDouble-click to reset");
    m_spinBox->setToolTip(tooltip);
}

void LabeledSlider::mouseDoubleClickEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    reset();
    QWidget::mouseDoubleClickEvent(event);
}
