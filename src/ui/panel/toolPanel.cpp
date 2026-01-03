#include "toolPanel.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollBar>

#include "../../controller/ApplicationController.h"  // For StyleTransferType enum
#include "../widgets/modernButton.h"
#include "../widgets/modernCollapsible.h"
#include "../widgets/modernSlider.h"
#include "../widgets/modernToolButton.h"

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
    separator->setStyleSheet("background: #333; max-height: 1px;");
    mainLayout->addWidget(separator);

    // Create scroll area for the content
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea { background: #1e1e1e; }
        QScrollBar:vertical {
            background: #1e1e1e;
            width: 6px;
            margin: 0;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #404040;
            border-radius: 3px;
            min-height: 40px;
        }
        QScrollBar::handle:vertical:hover { background: #505050; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0; background: none;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )");

    // Content widget
    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background: #1e1e1e;");
    m_mainLayout = new QVBoxLayout(m_contentWidget);
    m_mainLayout->setContentsMargins(0, 8, 0, 16);
    m_mainLayout->setSpacing(4);

    // Create collapsible sections in logical order
    m_mainLayout->addWidget(createPresetsSection());  // Presets at top
    m_mainLayout->addWidget(createEffectsSection());  // Filters first (most used)
    m_mainLayout->addWidget(createAIStyleSection());  // AI Style Transfer
    m_mainLayout->addWidget(createBasicSection());    // Light adjustments
    m_mainLayout->addWidget(createColorSection());    // Color adjustments
    m_mainLayout->addWidget(createToolSection());     // Geometry tools
    m_mainLayout->addWidget(createDetailSection());   // Detail/other
    m_mainLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
}

QWidget* ToolPanel::createQuickActionsBar() {
    QWidget* bar = new QWidget(this);
    bar->setStyleSheet("background: #252525; border-bottom: 1px solid #333;");
    bar->setFixedHeight(80);

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // Auto Enhance button
    m_autoEnhanceBtn = new ModernToolButton("Auto", ":/assets/icons/auto_enhance.png", bar);
    m_autoEnhanceBtn->setBadgeText("AI");
    m_autoEnhanceBtn->setToolTip("Auto-enhance image with AI");
    connect(m_autoEnhanceBtn, &ModernToolButton::clicked, this,
            &ToolPanel::filterAutoEnhanceRequested);

    // Apply button (applies corrections to permanent image)
    ModernToolButton* applyBtn = new ModernToolButton("Apply", ":/assets/icons/apply.png", bar);
    applyBtn->setToolTip("Apply current adjustments permanently");
    connect(applyBtn, &ModernToolButton::clicked, this, &ToolPanel::applyRequested);

    // Before/After compare button (checkable)
    m_compareBtn = new ModernToolButton("Compare", ":/assets/icons/compare.png", bar);
    m_compareBtn->setCheckable(true);
    m_compareBtn->setToolTip("Toggle before/after comparison (Space)");
    connect(m_compareBtn, &ModernToolButton::toggled, this, &ToolPanel::compareModeToggled);

    // Zoom button (toggleable)
    m_zoomBtn = new ModernToolButton("Zoom", ":/assets/icons/zoom_in.png", bar);
    m_zoomBtn->setCheckable(true);
    m_zoomBtn->setToolTip(
        "Toggle Zoom mode (Z)\nClick = zoom in, Alt+Click = zoom out\nScroll to zoom");
    connect(m_zoomBtn, &ModernToolButton::toggled, this, &ToolPanel::zoomModeToggled);

    // Save Preset button
    ModernToolButton* savePresetBtn = new ModernToolButton("Preset", ":/assets/icons/save.png", bar);
    savePresetBtn->setToolTip("Save current settings as preset");
    connect(savePresetBtn, &ModernToolButton::clicked, this, &ToolPanel::savePresetButtonClicked);

    // Reset button
    ModernToolButton* resetBtn = new ModernToolButton("Reset", ":/assets/icons/reset.png", bar);
    resetBtn->setToolTip("Reset all adjustments");
    connect(resetBtn, &ModernToolButton::clicked, this, &ToolPanel::resetAllAdjustments);

    layout->addWidget(m_autoEnhanceBtn);
    layout->addWidget(applyBtn);
    layout->addWidget(m_compareBtn);
    layout->addWidget(m_zoomBtn);
    layout->addWidget(savePresetBtn);
    layout->addStretch();
    layout->addWidget(resetBtn);

    return bar;
}

ModernCollapsible* ToolPanel::createPresetsSection() {
    ModernCollapsible* presetsSection = new ModernCollapsible("Presets", "🎛️", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 12);

    // Info label
    QLabel* infoLabel = new QLabel(
        "<span style='color: #888; font-size: 10px;'>"
        "Apply pre-configured adjustment styles</span>",
        this);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    // Preset combo box
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem("Select a preset...");
    m_presetCombo->insertSeparator(1);

    // Add built-in presets
    QStringList builtInPresets = {"Cinematic", "Portrait",     "Landscape",     "Warm Sunset",
                                  "Cool Blue", "Vintage Film", "High Contrast", "Soft Light",
                                  "Dramatic",  "Natural"};
    for (const QString& preset : builtInPresets) {
        m_presetCombo->addItem("📦 " + preset, preset);
    }

    m_presetCombo->setStyleSheet(R"(
        QComboBox {
            background: #2a2a2a;
            border: 1px solid #3a3a3a;
            border-radius: 6px;
            padding: 10px 12px;
            color: #d0d0d0;
            font-size: 12px;
            min-height: 20px;
        }
        QComboBox:hover { border-color: #4a4a4a; background: #303030; }
        QComboBox:focus { border-color: #6366f1; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox::down-arrow { image: url(:/assets/icons/dropdown.png); width: 10px; }
        QComboBox QAbstractItemView {
            background: #2a2a2a;
            color: #d0d0d0;
            selection-background-color: #6366f1;
            selection-color: white;
            border: 1px solid #3a3a3a;
            border-radius: 6px;
            padding: 4px;
        }
        QComboBox QAbstractItemView::item {
            padding: 6px 12px;
            min-height: 24px;
        }
        QComboBox QAbstractItemView::item:hover {
            background: #3a3a3a;
        }
    )");

    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                if (index <= 1)
                    return;  // Skip "Select..." and separator
                QString presetName = m_presetCombo->itemData(index).toString();
                if (!presetName.isEmpty()) {
                    Q_EMIT presetSelected(presetName);
                }
            });

    layout->addWidget(m_presetCombo);

    // Save preset button
    IconButton* saveBtn = new IconButton("", "Save Current as Preset", this);
    saveBtn->setIconEmoji("💾");
    saveBtn->setToolTip("Save current adjustments as a new preset");
    connect(saveBtn, &QPushButton::clicked, this, &ToolPanel::savePresetButtonClicked);
    layout->addWidget(saveBtn);

    presetsSection->setContentLayout(layout);
    return presetsSection;
}

ModernCollapsible* ToolPanel::createBasicSection() {
    ModernCollapsible* basicSection = new ModernCollapsible("Light", "☀️", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(8, 8, 8, 12);

    // Create modern sliders
    m_exposureSlider = new ModernSlider("Exposure", -100, 100, 0, this);
    m_exposureSlider->setTooltip("Adjust overall brightness\nDouble-click to reset");

    m_contrastSlider = new ModernSlider("Contrast", -100, 100, 0, this);
    m_contrastSlider->setTooltip(
        "Adjust the difference between light and dark\nDouble-click to reset");

    m_highlightsSlider = new ModernSlider("Highlights", -100, 100, 0, this);
    m_highlightsSlider->setTooltip("Recover or brighten bright areas\nDouble-click to reset");

    m_shadowsSlider = new ModernSlider("Shadows", -100, 100, 0, this);
    m_shadowsSlider->setTooltip("Brighten or darken shadow areas\nDouble-click to reset");

    m_whitesSlider = new ModernSlider("Whites", -100, 100, 0, this);
    m_whitesSlider->setTooltip("Adjust white point and bright tones\nDouble-click to reset");

    m_blacksSlider = new ModernSlider("Blacks", -100, 100, 0, this);
    m_blacksSlider->setTooltip("Adjust black point and dark tones\nDouble-click to reset");

    // Connect signals
    connect(m_exposureSlider, &ModernSlider::valueChanged, this, &ToolPanel::exposureChanged);
    connect(m_contrastSlider, &ModernSlider::valueChanged, this, &ToolPanel::contrastChanged);
    connect(m_highlightsSlider, &ModernSlider::valueChanged, this, &ToolPanel::highlightsChanged);
    connect(m_shadowsSlider, &ModernSlider::valueChanged, this, &ToolPanel::shadowsChanged);
    connect(m_whitesSlider, &ModernSlider::valueChanged, this, &ToolPanel::whitesChanged);
    connect(m_blacksSlider, &ModernSlider::valueChanged, this, &ToolPanel::blacksChanged);

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

ModernCollapsible* ToolPanel::createColorSection() {
    ModernCollapsible* colorSection = new ModernCollapsible("Color", "🎨", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(8, 8, 8, 12);

    // Create modern sliders
    m_temperatureSlider = new ModernSlider("Temperature", -100, 100, 0, this);
    m_temperatureSlider->setTooltip(
        "Adjust color temperature\nCooler (blue) ← → Warmer (yellow)\nDouble-click to reset");

    m_tintSlider = new ModernSlider("Tint", -100, 100, 0, this);
    m_tintSlider->setTooltip(
        "Adjust green-magenta balance\nGreen ← → Magenta\nDouble-click to reset");

    m_saturationSlider = new ModernSlider("Saturation", -100, 100, 0, this);
    m_saturationSlider->setTooltip("Adjust color intensity\nDouble-click to reset");

    // Connect signals
    connect(m_temperatureSlider, &ModernSlider::valueChanged, this, &ToolPanel::temperatureChanged);
    connect(m_tintSlider, &ModernSlider::valueChanged, this, &ToolPanel::tintChanged);
    connect(m_saturationSlider, &ModernSlider::valueChanged, this, &ToolPanel::saturationChanged);

    // Add to layout
    layout->addWidget(m_temperatureSlider);
    layout->addWidget(m_tintSlider);
    layout->addWidget(m_saturationSlider);

    colorSection->setContentLayout(layout);
    return colorSection;
}

ModernCollapsible* ToolPanel::createDetailSection() {
    ModernCollapsible* detailSection = new ModernCollapsible("Detail", "✨", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(8, 8, 8, 12);

    // Create modern slider
    m_brightnessSlider = new ModernSlider("Brightness", -100, 100, 0, this);
    m_brightnessSlider->setTooltip("Adjust overall brightness\nDouble-click to reset");

    // Connect signals
    connect(m_brightnessSlider, &ModernSlider::valueChanged, this, &ToolPanel::brightnessChanged);

    // Add to layout
    layout->addWidget(m_brightnessSlider);

    detailSection->setContentLayout(layout);
    detailSection->setExpanded(false);  // Start collapsed
    return detailSection;
}

ModernCollapsible* ToolPanel::createEffectsSection() {
    ModernCollapsible* effectsSection = new ModernCollapsible("Effects", "✨", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 12);

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

ModernCollapsible* ToolPanel::createAIStyleSection() {
    ModernCollapsible* styleSection = new ModernCollapsible("AI Style Transfer", "🎨", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(8);
    layout->setContentsMargins(8, 8, 8, 12);

    // Info label
    QLabel* infoLabel = new QLabel(
        "<span style='color: #666; font-size: 10px;'>"
        "Transform your photo using neural networks</span>",
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
                StyleTransferType styleType = StyleTransferType::Mosaic;
                if (styleName == "Mosaic")
                    styleType = StyleTransferType::Mosaic;
                else if (styleName == "Candy")
                    styleType = StyleTransferType::Candy;
                else if (styleName == "Rain Princess")
                    styleType = StyleTransferType::RainPrincess;
                else if (styleName == "Udnie")
                    styleType = StyleTransferType::Udnie;
                else if (styleName == "Pointillism")
                    styleType = StyleTransferType::Pointillism;
                Q_EMIT styleTransferRequested(styleType);
            });

    layout->addWidget(m_styleGallery);

    // Processing indicator (hidden by default)
    QLabel* processingLabel = new QLabel(
        "<span style='color: #6366f1; font-size: 10px;'>"
        "⏳ Processing... AI style transfer may take a few seconds</span>",
        this);
    processingLabel->setWordWrap(true);
    processingLabel->setObjectName("processingLabel");
    processingLabel->hide();
    layout->addWidget(processingLabel);

    styleSection->setContentLayout(layout);
    styleSection->setExpanded(false);  // Start collapsed
    return styleSection;
}

void ToolPanel::resetAllAdjustments() {
    // Reset all modern sliders to default values
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

ModernCollapsible* ToolPanel::createToolSection() {
    ModernCollapsible* toolSection = new ModernCollapsible("Transform", "📐", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(12);
    layout->setContentsMargins(8, 8, 8, 12);

    // Rotate tools
    QLabel* rotateLabel = new QLabel("ROTATE", this);
    rotateLabel->setStyleSheet(
        "font-weight: 600; color: #666; font-size: 10px; letter-spacing: 1px;");
    layout->addWidget(rotateLabel);

    QHBoxLayout* rotateLayout = new QHBoxLayout();
    rotateLayout->setSpacing(8);

    IconButton* rotateLeftBtn =
        new IconButton(":/assets/icons/rotate_left.png", "Rotate Left", this);
    rotateLeftBtn->setIconEmoji("↺");
    connect(rotateLeftBtn, &QPushButton::clicked, this, &ToolPanel::rotateLeftRequested);

    IconButton* rotateRightBtn =
        new IconButton(":/assets/icons/rotate_right.png", "Rotate Right", this);
    rotateRightBtn->setIconEmoji("↻");
    connect(rotateRightBtn, &QPushButton::clicked, this, &ToolPanel::rotateRightRequested);

    rotateLayout->addWidget(rotateLeftBtn);
    rotateLayout->addWidget(rotateRightBtn);
    rotateLayout->addStretch();
    layout->addLayout(rotateLayout);

    // Flip tools
    QLabel* flipLabel = new QLabel("FLIP", this);
    flipLabel->setStyleSheet(
        "font-weight: 600; color: #666; font-size: 10px; letter-spacing: 1px;");
    layout->addWidget(flipLabel);

    QHBoxLayout* flipLayout = new QHBoxLayout();
    flipLayout->setSpacing(8);

    IconButton* flipHBtn =
        new IconButton(":/assets/icons/flip_horizontal.png", "Flip Horizontal", this);
    flipHBtn->setIconEmoji("↔");
    connect(flipHBtn, &QPushButton::clicked, this, &ToolPanel::flipHorizontalRequested);

    IconButton* flipVBtn =
        new IconButton(":/assets/icons/flip_vertical.png", "Flip Vertical", this);
    flipVBtn->setIconEmoji("↕");
    connect(flipVBtn, &QPushButton::clicked, this, &ToolPanel::flipVerticalRequested);

    flipLayout->addWidget(flipHBtn);
    flipLayout->addWidget(flipVBtn);
    flipLayout->addStretch();
    layout->addLayout(flipLayout);

    // Crop section
    QLabel* cropLabel = new QLabel("CROP", this);
    cropLabel->setStyleSheet(
        "font-weight: 600; color: #666; font-size: 10px; letter-spacing: 1px; margin-top: 4px;");
    layout->addWidget(cropLabel);

    // Crop button to activate crop mode
    IconButton* cropBtn = new IconButton(":/assets/icons/crop.png", "Start Crop", this);
    cropBtn->setIconEmoji("✂️");
    cropBtn->setToolTip("Click to enter crop mode, then drag on image to select area");
    connect(cropBtn, &QPushButton::clicked, this, &ToolPanel::cropRequested);

    QHBoxLayout* cropBtnLayout = new QHBoxLayout();
    cropBtnLayout->addWidget(cropBtn);
    cropBtnLayout->addStretch();
    layout->addLayout(cropBtnLayout);

    // Crop mode options
    QLabel* cropOptionsLabel = new QLabel("ASPECT RATIO", this);
    cropOptionsLabel->setStyleSheet(
        "font-weight: 600; color: #666; font-size: 10px; letter-spacing: 1px; margin-top: 4px;");
    layout->addWidget(cropOptionsLabel);

    m_cropModeCombo = new QComboBox(this);
    m_cropModeCombo->addItem("Free", 0);
    m_cropModeCombo->addItem("1:1 Square", 1);
    m_cropModeCombo->addItem("4:3 Photo", 2);
    m_cropModeCombo->addItem("3:2 Photo", 3);
    m_cropModeCombo->addItem("16:9 Widescreen", 4);
    m_cropModeCombo->addItem("21:9 Ultrawide", 5);
    m_cropModeCombo->addItem("3:4 Portrait", 6);
    m_cropModeCombo->addItem("2:3 Portrait", 7);
    m_cropModeCombo->addItem("9:16 Portrait", 8);
    m_cropModeCombo->addItem("Fixed Size...", 100);
    m_cropModeCombo->setStyleSheet(R"(
        QComboBox {
            background: #2a2a2a;
            border: 1px solid #3a3a3a;
            border-radius: 6px;
            padding: 8px 12px;
            color: #d0d0d0;
            font-size: 12px;
        }
        QComboBox:hover { border-color: #4a4a4a; background: #303030; }
        QComboBox:focus { border-color: #0078d4; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox::down-arrow { image: url(:/assets/icons/dropdown.png); width: 10px; }
        QComboBox QAbstractItemView {
            background: #2a2a2a;
            color: #d0d0d0;
            selection-background-color: #0078d4;
            selection-color: white;
            border: 1px solid #3a3a3a;
            border-radius: 6px;
            padding: 4px;
        }
    )");
    layout->addWidget(m_cropModeCombo);

    // Fixed size options (initially hidden)
    m_fixedSizeWidget = new QWidget(this);
    QHBoxLayout* fixedSizeLayout = new QHBoxLayout(m_fixedSizeWidget);
    fixedSizeLayout->setContentsMargins(0, 8, 0, 0);
    fixedSizeLayout->setSpacing(8);

    m_cropWidthSpin = new QSpinBox(this);
    m_cropWidthSpin->setRange(1, 10000);
    m_cropWidthSpin->setValue(800);
    m_cropWidthSpin->setSuffix(" px");
    m_cropWidthSpin->setStyleSheet(R"(
        QSpinBox {
            background: #2a2a2a;
            border: 1px solid #3a3a3a;
            border-radius: 6px;
            padding: 6px 8px;
            color: #d0d0d0;
            font-size: 12px;
        }
        QSpinBox:hover { border-color: #4a4a4a; }
        QSpinBox:focus { border-color: #0078d4; }
    )");

    QLabel* xLabel = new QLabel("×", this);
    xLabel->setStyleSheet("color: #666; font-size: 14px; font-weight: bold;");

    m_cropHeightSpin = new QSpinBox(this);
    m_cropHeightSpin->setRange(1, 10000);
    m_cropHeightSpin->setValue(600);
    m_cropHeightSpin->setSuffix(" px");
    m_cropHeightSpin->setStyleSheet(m_cropWidthSpin->styleSheet());

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
    toolSection->setExpanded(false);  // Start collapsed
    return toolSection;
}

void ToolPanel::setZoomModeChecked(bool checked) {
    // Block signals to avoid recursive updates
    m_zoomBtn->blockSignals(true);
    m_zoomBtn->setChecked(checked);
    m_zoomBtn->blockSignals(false);
}

void ToolPanel::refreshPresets(const QStringList& userPresets) {
    if (!m_presetCombo) return;
    
    // Block signals during refresh
    m_presetCombo->blockSignals(true);
    
    // Remember current selection
    QString currentSelection = m_presetCombo->currentData().toString();
    
    // Clear and rebuild
    m_presetCombo->clear();
    m_presetCombo->addItem("Select a preset...");
    m_presetCombo->insertSeparator(1);
    
    // Add built-in presets
    QStringList builtInPresets = {"Cinematic", "Portrait", "Landscape", "Warm Sunset",
                                  "Cool Blue", "Vintage Film", "High Contrast", "Soft Light",
                                  "Dramatic", "Natural"};
    for (const QString& preset : builtInPresets) {
        m_presetCombo->addItem("📦 " + preset, preset);
    }
    
    // Add separator if there are user presets
    if (!userPresets.isEmpty()) {
        m_presetCombo->insertSeparator(m_presetCombo->count());
        
        // Add user presets
        for (const QString& preset : userPresets) {
            m_presetCombo->addItem("👤 " + preset, preset);
        }
    }
    
    // Restore selection if it still exists
    if (!currentSelection.isEmpty()) {
        int index = m_presetCombo->findData(currentSelection);
        if (index >= 0) {
            m_presetCombo->setCurrentIndex(index);
        }
    }
    
    m_presetCombo->blockSignals(false);
}

void ToolPanel::updateSliders(int brightness, int contrast, int saturation, int exposure,
                              int highlights, int shadows, int whites, int blacks,
                              int temperature, int tint) {
    // Update all sliders without triggering valueChanged signals
    m_brightnessSlider->setValue(brightness);
    m_contrastSlider->setValue(contrast);
    m_saturationSlider->setValue(saturation);
    m_exposureSlider->setValue(exposure);
    m_highlightsSlider->setValue(highlights);
    m_shadowsSlider->setValue(shadows);
    m_whitesSlider->setValue(whites);
    m_blacksSlider->setValue(blacks);
    m_temperatureSlider->setValue(temperature);
    m_tintSlider->setValue(tint);
}