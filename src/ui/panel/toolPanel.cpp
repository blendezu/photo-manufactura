#include "toolPanel.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollBar>

#include "../widgets/collapsibleWidget.h"
#include "../widgets/labeledSlider.h"
#include "../widgets/modernToolButton.h"
#include "../widgets/toolPaletteWidget.h"

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

    // Quick Actions Bar at the top (always visible)
    mainLayout->addWidget(createQuickActionsBar());

    // Separator line
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background: #404040; max-height: 1px;");
    mainLayout->addWidget(separator);

    // Create scroll area for the content
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; }"
        "QScrollBar:vertical {"
        "  background: #2a2a2a; width: 8px; margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #505050; border-radius: 4px; min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #606060; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0; background: none;"
        "}");

    // Content widget
    m_contentWidget = new QWidget();
    m_mainLayout = new QVBoxLayout(m_contentWidget);
    m_mainLayout->setContentsMargins(8, 12, 8, 12);
    m_mainLayout->setSpacing(8);

    // Create collapsible sections in logical order
    m_mainLayout->addWidget(createEffectsSection());  // Filters first (most used)
    m_mainLayout->addWidget(createAIStyleSection());  // AI Style Transfer
    m_mainLayout->addWidget(createToolSection());     // Geometry tools
    m_mainLayout->addWidget(createBasicSection());    // Light adjustments
    m_mainLayout->addWidget(createColorSection());    // Color adjustments
    m_mainLayout->addWidget(createDetailSection());   // Detail/other
    m_mainLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
}

QWidget* ToolPanel::createQuickActionsBar() {
    QWidget* bar = new QWidget(this);
    bar->setStyleSheet("background: #2d2d2d;");
    bar->setFixedHeight(90);

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(8);

    // Auto Enhance button
    m_autoEnhanceBtn = new ModernToolButton("Auto", ":/assets/icons/auto_enhance.png", bar);
    m_autoEnhanceBtn->setBadgeText("AI");
    m_autoEnhanceBtn->setToolTip("Auto-enhance image with AI");
    connect(m_autoEnhanceBtn, &ModernToolButton::clicked, this,
            &ToolPanel::filterAutoEnhanceRequested);

    // Crop button
    m_cropBtn = new ModernToolButton("Crop", ":/assets/icons/crop.png", bar);
    m_cropBtn->setToolTip("Crop image");
    connect(m_cropBtn, &ModernToolButton::clicked, this, &ToolPanel::cropRequested);

    // Rotate button
    m_rotateBtn = new ModernToolButton("Rotate", ":/assets/icons/rotate_right.png", bar);
    m_rotateBtn->setToolTip("Rotate image 90°");
    connect(m_rotateBtn, &ModernToolButton::clicked, this, &ToolPanel::rotateRightRequested);

    // Reset button
    ModernToolButton* resetBtn = new ModernToolButton("Reset", ":/assets/icons/reset.png", bar);
    resetBtn->setToolTip("Reset all adjustments");
    connect(resetBtn, &ModernToolButton::clicked, this, &ToolPanel::resetAllAdjustments);

    layout->addWidget(m_autoEnhanceBtn);
    layout->addWidget(m_cropBtn);
    layout->addWidget(m_rotateBtn);
    layout->addStretch();
    layout->addWidget(resetBtn);

    return bar;
}

CollapsibleWidget* ToolPanel::createBasicSection() {
    CollapsibleWidget* basicSection = new CollapsibleWidget("Basic Adjustments", this);
    QVBoxLayout* layout = new QVBoxLayout();

    layout->setSpacing(8);
    layout->setContentsMargins(5, 10, 5, 10);

    // Create sliders
    m_exposureSlider = new LabeledSlider("Exposure", -100, 100, 0, this);
    m_exposureSlider->setTooltip("Adjust overall brightness\nDouble-click to reset");

    m_contrastSlider = new LabeledSlider("Contrast", -100, 100, 0, this);
    m_contrastSlider->setTooltip(
        "Adjust the difference between light and dark\nDouble-click to reset");

    m_highlightsSlider = new LabeledSlider("Highlights", -100, 100, 0, this);
    m_highlightsSlider->setTooltip("Recover or brighten bright areas\nDouble-click to reset");

    m_shadowsSlider = new LabeledSlider("Shadows", -100, 100, 0, this);
    m_shadowsSlider->setTooltip("Brighten or darken shadow areas\nDouble-click to reset");

    m_whitesSlider = new LabeledSlider("Whites", -100, 100, 0, this);
    m_whitesSlider->setTooltip("Adjust white point and bright tones\nDouble-click to reset");

    m_blacksSlider = new LabeledSlider("Blacks", -100, 100, 0, this);
    m_blacksSlider->setTooltip("Adjust black point and dark tones\nDouble-click to reset");

    // Connect signals
    // TODO: MOVE TO CONTROLLER: These signals should connect to ApplicationController slots
    // The controller should handle:
    //   1. Receiving adjustment values
    //   2. Coordinating with ImageProcessingService to apply adjustments
    //   3. Managing state and undo/redo history
    //   4. Triggering image reprocessing via ImagePipeline
    // Example: connect(m_exposureSlider, &LabeledSlider::valueChanged,
    //                  controller, &ApplicationController::adjustExposure);
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
    basicSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return basicSection;
}

CollapsibleWidget* ToolPanel::createColorSection() {
    CollapsibleWidget* colorSection = new CollapsibleWidget("Color Adjustments", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(5, 10, 5, 10);

    // Create sliders
    m_temperatureSlider = new LabeledSlider("Temperature", -100, 100, 0, this);
    m_temperatureSlider->setTooltip(
        "Adjust color temperature\nCooler (blue) ← → Warmer (yellow)\nDouble-click to reset");

    m_tintSlider = new LabeledSlider("Tint", -100, 100, 0, this);
    m_tintSlider->setTooltip(
        "Adjust green-magenta balance\nGreen ← → Magenta\nDouble-click to reset");

    m_saturationSlider = new LabeledSlider("Saturation", -100, 100, 0, this);
    m_saturationSlider->setTooltip("Adjust color intensity\nDouble-click to reset");

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
    layout->setContentsMargins(5, 10, 5, 10);

    // Create sliders
    m_brightnessSlider = new LabeledSlider("Brightness", -100, 100, 0, this);
    m_brightnessSlider->setTooltip("Adjust overall brightness\nDouble-click to reset");

    // Connect signals
    connect(m_brightnessSlider, &LabeledSlider::valueChanged, this, &ToolPanel::brightnessChanged);

    // Add to layout
    layout->addWidget(m_brightnessSlider);

    detailSection->setContentLayout(layout);
    return detailSection;
}

CollapsibleWidget* ToolPanel::createEffectsSection() {
    CollapsibleWidget* effectsSection = new CollapsibleWidget("✨ Effects & Filters", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(5, 10, 5, 10);

    // Filter gallery with preview cards
    m_filterGallery = new FilterGalleryWidget(this);
    m_filterGallery->addFilter("Original", ":/assets/previews/original.png");
    m_filterGallery->addFilter("Grayscale", ":/assets/previews/grayscale.png");
    m_filterGallery->addFilter("Vintage", ":/assets/previews/vintage.png");
    m_filterGallery->setSelectedFilter("Original");

    connect(m_filterGallery, &FilterGalleryWidget::filterSelected, this,
            [this](const QString& filterName) {
                if (filterName == "Original") {
                    Q_EMIT filterOriginalRequested();
                } else if (filterName == "Grayscale") {
                    Q_EMIT filterGrayscaleRequested();
                } else if (filterName == "Vintage") {
                    Q_EMIT filterVintageRequested();
                }
            });

    layout->addWidget(m_filterGallery);

    effectsSection->setContentLayout(layout);
    return effectsSection;
}

CollapsibleWidget* ToolPanel::createAIStyleSection() {
    CollapsibleWidget* styleSection = new CollapsibleWidget("🎨 AI Style Transfer", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(5, 10, 5, 10);

    // Info label
    QLabel* infoLabel = new QLabel(
        "<span style='color: #888; font-size: 11px;'>"
        "Transform your photo into artistic styles using neural networks</span>",
        this);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    // Style gallery
    m_styleGallery = new FilterGalleryWidget(this);
    m_styleGallery->addFilter("Mosaic", ":/assets/previews/mosaic.png");
    m_styleGallery->addFilter("Candy", ":/assets/previews/candy.png");
    m_styleGallery->addFilter("Rain Princess", ":/assets/previews/rain_princess.png");
    m_styleGallery->addFilter("Udnie", ":/assets/previews/udnie.png");
    m_styleGallery->addFilter("Pointillism", ":/assets/previews/pointillism.png");

    connect(m_styleGallery, &FilterGalleryWidget::filterSelected, this,
            [this](const QString& styleName) {
                int styleType = 0;
                if (styleName == "Mosaic")
                    styleType = 0;
                else if (styleName == "Candy")
                    styleType = 1;
                else if (styleName == "Rain Princess")
                    styleType = 2;
                else if (styleName == "Udnie")
                    styleType = 3;
                else if (styleName == "Pointillism")
                    styleType = 4;
                Q_EMIT styleTransferRequested(styleType);
            });

    layout->addWidget(m_styleGallery);

    // Processing indicator (hidden by default)
    QLabel* processingLabel = new QLabel(
        "<span style='color: #6366f1; font-size: 11px;'>"
        "⏳ Processing... AI style transfer may take a few seconds</span>",
        this);
    processingLabel->setWordWrap(true);
    processingLabel->setObjectName("processingLabel");
    processingLabel->hide();
    layout->addWidget(processingLabel);

    styleSection->setContentLayout(layout);
    return styleSection;
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

    // Notify controller to reset processing parameters
    Q_EMIT resetAllRequested();
}

CollapsibleWidget* ToolPanel::createToolSection() {
    CollapsibleWidget* toolSection = new CollapsibleWidget("Geometry Tools", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(12);
    layout->setContentsMargins(5, 10, 5, 10);

    // Rotate tools with icons
    QLabel* rotateLabel = new QLabel("Rotate", this);
    rotateLabel->setStyleSheet("font-weight: bold; color: #aaa;");
    layout->addWidget(rotateLabel);

    m_rotateToolPalette = new ToolPaletteWidget("Rotate Tools", this);
    m_rotateToolPalette->addToolButton("Rotate Left", ":/assets/icons/rotate_left.png");
    m_rotateToolPalette->addToolButton("Rotate Right", ":/assets/icons/rotate_right.png");
    connect(m_rotateToolPalette, &ToolPaletteWidget::toolActivated, this,
            [this](const QString& tool) {
                if (tool == "Rotate Left")
                    Q_EMIT rotateLeftRequested();
                else if (tool == "Rotate Right")
                    Q_EMIT rotateRightRequested();
            });
    layout->addWidget(m_rotateToolPalette);

    // Flip tools with icons
    QLabel* flipLabel = new QLabel("Flip", this);
    flipLabel->setStyleSheet("font-weight: bold; color: #aaa;");
    layout->addWidget(flipLabel);

    m_flipToolPalette = new ToolPaletteWidget("Flip Tools", this);
    m_flipToolPalette->addToolButton("Flip Horizontal", ":/assets/icons/flip_horizontal.png");
    m_flipToolPalette->addToolButton("Flip Vertical", ":/assets/icons/flip_vertical.png");
    connect(m_flipToolPalette, &ToolPaletteWidget::toolActivated, this,
            [this](const QString& tool) {
                if (tool == "Flip Horizontal")
                    Q_EMIT flipHorizontalRequested();
                else if (tool == "Flip Vertical")
                    Q_EMIT flipVerticalRequested();
            });
    layout->addWidget(m_flipToolPalette);

    // Crop tool with icon
    QLabel* cropLabel = new QLabel("Crop", this);
    cropLabel->setStyleSheet("font-weight: bold; color: #aaa;");
    layout->addWidget(cropLabel);

    m_cropToolPalette = new ToolPaletteWidget("Crop Tools", this);
    m_cropToolPalette->addToolButton("Crop", ":/assets/icons/crop.png");
    m_cropToolPalette->addToolButton("Straighten", ":/assets/icons/straighten.png");
    connect(m_cropToolPalette, &ToolPaletteWidget::toolActivated, this,
            [this](const QString& tool) {
                if (tool == "Crop")
                    Q_EMIT cropRequested();
                else if (tool == "Straighten")
                    Q_EMIT straightenRequested();
            });
    layout->addWidget(m_cropToolPalette);

    // Crop options - Aspect Ratio / Fixed Size
    QLabel* cropOptionsLabel = new QLabel("Crop Mode", this);
    cropOptionsLabel->setStyleSheet("font-weight: bold; color: #aaa; margin-top: 8px;");
    layout->addWidget(cropOptionsLabel);

    m_cropModeCombo = new QComboBox(this);
    m_cropModeCombo->addItem("Free", 0);             // AspectRatioPreset::Free
    m_cropModeCombo->addItem("1:1 Square", 1);       // AspectRatioPreset::Square_1_1
    m_cropModeCombo->addItem("4:3 Photo", 2);        // AspectRatioPreset::Photo_4_3
    m_cropModeCombo->addItem("3:2 Photo", 3);        // AspectRatioPreset::Photo_3_2
    m_cropModeCombo->addItem("16:9 Widescreen", 4);  // AspectRatioPreset::Widescreen_16_9
    m_cropModeCombo->addItem("21:9 Ultrawide", 5);   // AspectRatioPreset::Widescreen_21_9
    m_cropModeCombo->addItem("3:4 Portrait", 6);     // AspectRatioPreset::Portrait_3_4
    m_cropModeCombo->addItem("2:3 Portrait", 7);     // AspectRatioPreset::Portrait_2_3
    m_cropModeCombo->addItem("9:16 Portrait", 8);    // AspectRatioPreset::Portrait_9_16
    m_cropModeCombo->addItem("Fixed Size...", 100);  // Special: Fixed size
    m_cropModeCombo->setStyleSheet(
        "QComboBox { background: #3a3a3a; border: 1px solid #555; border-radius: 3px; "
        "padding: 4px 8px; color: #ddd; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox::down-arrow { image: url(:/assets/icons/dropdown.png); width: 12px; }"
        "QComboBox QAbstractItemView { background: #3a3a3a; color: #ddd; "
        "selection-background-color: #555; }");
    layout->addWidget(m_cropModeCombo);

    // Fixed size options (initially hidden)
    m_fixedSizeWidget = new QWidget(this);
    QHBoxLayout* fixedSizeLayout = new QHBoxLayout(m_fixedSizeWidget);
    fixedSizeLayout->setContentsMargins(0, 4, 0, 0);
    fixedSizeLayout->setSpacing(4);

    m_cropWidthSpin = new QSpinBox(this);
    m_cropWidthSpin->setRange(1, 10000);
    m_cropWidthSpin->setValue(800);
    m_cropWidthSpin->setSuffix(" px");
    m_cropWidthSpin->setStyleSheet(
        "QSpinBox { background: #3a3a3a; border: 1px solid #555; border-radius: 3px; "
        "padding: 2px 4px; color: #ddd; }");

    QLabel* xLabel = new QLabel("×", this);
    xLabel->setStyleSheet("color: #aaa;");

    m_cropHeightSpin = new QSpinBox(this);
    m_cropHeightSpin->setRange(1, 10000);
    m_cropHeightSpin->setValue(600);
    m_cropHeightSpin->setSuffix(" px");
    m_cropHeightSpin->setStyleSheet(
        "QSpinBox { background: #3a3a3a; border: 1px solid #555; border-radius: 3px; "
        "padding: 2px 4px; color: #ddd; }");

    fixedSizeLayout->addWidget(m_cropWidthSpin);
    fixedSizeLayout->addWidget(xLabel);
    fixedSizeLayout->addWidget(m_cropHeightSpin);
    fixedSizeLayout->addStretch();
    m_fixedSizeWidget->setVisible(false);
    layout->addWidget(m_fixedSizeWidget);

    // Connect crop mode combo
    connect(m_cropModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                int presetValue = m_cropModeCombo->itemData(index).toInt();
                if (presetValue == 100) {
                    // Fixed size mode
                    m_fixedSizeWidget->setVisible(true);
                    Q_EMIT cropFixedSizeChanged(m_cropWidthSpin->value(),
                                                m_cropHeightSpin->value());
                } else {
                    m_fixedSizeWidget->setVisible(false);
                    Q_EMIT cropAspectRatioChanged(presetValue);
                }
            });

    // Connect fixed size spinboxes
    connect(m_cropWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        if (m_fixedSizeWidget->isVisible()) {
            Q_EMIT cropFixedSizeChanged(m_cropWidthSpin->value(), m_cropHeightSpin->value());
        }
    });
    connect(m_cropHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        if (m_fixedSizeWidget->isVisible()) {
            Q_EMIT cropFixedSizeChanged(m_cropWidthSpin->value(), m_cropHeightSpin->value());
        }
    });

    toolSection->setContentLayout(layout);
    return toolSection;
}