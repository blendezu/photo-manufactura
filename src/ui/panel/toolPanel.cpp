#include "toolPanel.h"

#include <QLabel>
#include <QPushButton>

#include "../widgets/collapsibleWidget.h"
#include "../widgets/labeledSlider.h"

ToolPanel::ToolPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

ToolPanel::~ToolPanel() {
    // Qt handles child widget deletion
}

void ToolPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create scroll area for the content
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Content widget
    m_contentWidget = new QWidget();
    m_mainLayout = new QVBoxLayout(m_contentWidget);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    // Title and Reset button
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* title = new QLabel(tr("<b>Adjustments</b>"), this);
    QPushButton* resetBtn = new QPushButton(tr("Reset All"), this);
    resetBtn->setMaximumWidth(80);
    connect(resetBtn, &QPushButton::clicked, this, &ToolPanel::resetAllAdjustments);
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    headerLayout->addWidget(resetBtn);
    m_mainLayout->addLayout(headerLayout);
    m_mainLayout->addSpacing(10);

    // Create collapsible sections
    m_mainLayout->addWidget(createBasicSection());
    m_mainLayout->addWidget(createColorSection());
    m_mainLayout->addWidget(createDetailSection());

    // Add stretch at the end
    m_mainLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
}

CollapsibleWidget* ToolPanel::createBasicSection() {
    CollapsibleWidget* basicSection = new CollapsibleWidget("Basic Adjustments", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(10, 10, 10, 10);

    // Create sliders
    m_exposureSlider = new LabeledSlider("Exposure", -100, 100, 0, this);
    m_contrastSlider = new LabeledSlider("Contrast", -100, 100, 0, this);
    m_highlightsSlider = new LabeledSlider("Highlights", -100, 100, 0, this);
    m_shadowsSlider = new LabeledSlider("Shadows", -100, 100, 0, this);
    m_whitesSlider = new LabeledSlider("Whites", -100, 100, 0, this);
    m_blacksSlider = new LabeledSlider("Blacks", -100, 100, 0, this);

    // Connect signals
    connect(m_exposureSlider, &LabeledSlider::valueChanged, this, &ToolPanel::exposureChanged);
    connect(m_contrastSlider, &LabeledSlider::valueChanged, this, &ToolPanel::contrastChanged);
    connect(m_highlightsSlider, &LabeledSlider::valueChanged, this, &ToolPanel::highlightsChanged);
    connect(m_shadowsSlider, &LabeledSlider::valueChanged, this, &ToolPanel::shadowsChanged);
    connect(m_whitesSlider, &LabeledSlider::valueChanged, this, &ToolPanel::whitesChanged);
    connect(m_blacksSlider, &LabeledSlider::valueChanged, this, &ToolPanel::blacksChanged);

    // Add to layout
    layout->addWidget(m_exposureSlider);
    layout->addWidget(m_contrastSlider);
    layout->addWidget(m_highlightsSlider);
    layout->addWidget(m_shadowsSlider);
    layout->addWidget(m_whitesSlider);
    layout->addWidget(m_blacksSlider);

    basicSection->setContentLayout(layout);
    return basicSection;
}

CollapsibleWidget* ToolPanel::createColorSection() {
    CollapsibleWidget* colorSection = new CollapsibleWidget("Color Adjustments", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(10, 10, 10, 10);

    // Create sliders
    m_temperatureSlider = new LabeledSlider("Temperature", -100, 100, 0, this);
    m_tintSlider = new LabeledSlider("Tint", -100, 100, 0, this);
    m_saturationSlider = new LabeledSlider("Saturation", -100, 100, 0, this);

    // Connect signals
    connect(m_temperatureSlider, &LabeledSlider::valueChanged, this,
            &ToolPanel::temperatureChanged);
    connect(m_tintSlider, &LabeledSlider::valueChanged, this, &ToolPanel::tintChanged);
    connect(m_saturationSlider, &LabeledSlider::valueChanged, this, &ToolPanel::saturationChanged);

    // Add to layout
    layout->addWidget(m_temperatureSlider);
    layout->addWidget(m_tintSlider);
    layout->addWidget(m_saturationSlider);

    colorSection->setContentLayout(layout);
    return colorSection;
}

CollapsibleWidget* ToolPanel::createDetailSection() {
    CollapsibleWidget* detailSection = new CollapsibleWidget("Detail", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(10, 10, 10, 10);

    // Create sliders
    m_brightnessSlider = new LabeledSlider("Brightness", -100, 100, 0, this);

    // Connect signals
    connect(m_brightnessSlider, &LabeledSlider::valueChanged, this, &ToolPanel::brightnessChanged);

    // Add to layout
    layout->addWidget(m_brightnessSlider);

    detailSection->setContentLayout(layout);
    return detailSection;
}

void ToolPanel::resetAllAdjustments() {
    // Reset all sliders to default values
    m_exposureSlider->reset();
    m_contrastSlider->reset();
    m_highlightsSlider->reset();
    m_shadowsSlider->reset();
    m_whitesSlider->reset();
    m_blacksSlider->reset();
    m_temperatureSlider->reset();
    m_tintSlider->reset();
    m_saturationSlider->reset();
    m_brightnessSlider->reset();
}
