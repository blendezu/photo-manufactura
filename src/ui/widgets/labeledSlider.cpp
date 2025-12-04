#include "labeledSlider.h"

#include <QPushButton>

LabeledSlider::LabeledSlider(const QString& label, int min, int max, int defaultValue, QWidget* parent) : QWidget(parent), m_defaultValue(defaultValue) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 12, 10, 12);
    mainLayout->setSpacing(8);

    // Label
    m_label = new QLabel(label, this);
    m_label->setStyleSheet("font-weight: bold; color: #8ad618; font-size: 13px;");
    m_label->setMinimumHeight(20);
    m_label->setMinimumWidth(100);
    m_label->setWordWrap(false);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Horizontal layout for slider and spinbox
    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(10);

    // Slider
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimum(min);
    m_slider->setMaximum(max);
    m_slider->setValue(defaultValue);
    m_slider->setTickPosition(QSlider::TicksBelow);
    m_slider->setTickInterval((max - min) / 4);
    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_slider->setMinimumHeight(24);

    // SpinBox
    m_spinBox = new QSpinBox(this);
    m_spinBox->setMinimum(min);
    m_spinBox->setMaximum(max);
    m_spinBox->setValue(defaultValue);
    m_spinBox->setFixedWidth(65);
    m_spinBox->setMinimumHeight(26);
    m_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_spinBox->setAlignment(Qt::AlignCenter);

    controlLayout->addWidget(m_slider);
    controlLayout->addWidget(m_spinBox);

    mainLayout->addWidget(m_label);
    mainLayout->addLayout(controlLayout);

    // Set minimum size to ensure visibility
    setMinimumHeight(60);
    setMinimumWidth(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Connect signals
    connect(m_slider, &QSlider::valueChanged, this, &LabeledSlider::onSliderChanged);
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabeledSlider::onSpinBoxChanged);
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
