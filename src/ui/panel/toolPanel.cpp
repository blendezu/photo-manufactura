#include "toolPanel.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollBar>

#include "../../controller/ApplicationController.h"  // For StyleTransferType enum
#include "../widgets/ResizeDialog.h"
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

    // Apply button (applies corrections to permanent image)
    ModernToolButton* applyBtn = new ModernToolButton("Apply", ":/assets/icons/apply.svg", bar);
    applyBtn->setToolTip("Apply current adjustments permanently");
    connect(applyBtn, &ModernToolButton::clicked, this, &ToolPanel::applyRequested);

    // Before/After compare button (checkable)
    m_compareBtn = new ModernToolButton("Compare", ":/assets/icons/compare.svg", bar);
    m_compareBtn->setCheckable(true);
    m_compareBtn->setToolTip("Toggle before/after comparison (Space)");
    connect(m_compareBtn, &ModernToolButton::toggled, this, &ToolPanel::compareModeToggled);

    // Zoom button (toggleable)
    m_zoomBtn = new ModernToolButton("Zoom", ":/assets/icons/zoom_in.svg", bar);
    m_zoomBtn->setCheckable(true);
    m_zoomBtn->setToolTip(
        "Toggle Zoom mode (Z)\nClick = zoom in, Alt+Click = zoom out\nScroll to zoom");
    connect(m_zoomBtn, &ModernToolButton::toggled, this, &ToolPanel::zoomModeToggled);

    // Reset button
    ModernToolButton* resetBtn = new ModernToolButton("Reset", ":/assets/icons/reset.svg", bar);
    resetBtn->setToolTip("Reset to original image (removes all adjustments and transforms)");
    connect(resetBtn, &ModernToolButton::clicked, this, &ToolPanel::resetToOriginalRequested);

    layout->addWidget(applyBtn);
    layout->addWidget(m_compareBtn);
    layout->addWidget(m_zoomBtn);
    // Removed duplicate Save Preset button
    layout->addStretch();
    layout->addWidget(resetBtn);

    return bar;
}

ModernCollapsible* ToolPanel::createPresetsSection() {
    ModernCollapsible* presetsSection =
        new ModernCollapsible("Presets", ":/assets/icons/presets.png", this);
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

    // Container for combo and save button
    QWidget* container = new QWidget(this);
    QHBoxLayout* comboLayout = new QHBoxLayout(container);
    comboLayout->setContentsMargins(0, 0, 0, 0);
    comboLayout->setSpacing(8);

    // Preset combo box
    m_presetCombo = new QComboBox(this);
    m_presetCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_presetCombo->addItem("Select a preset...");
    m_presetCombo->insertSeparator(1);

    // Add built-in presets
    QStringList builtInPresets = {"Cinematic", "Portrait",     "Landscape",     "Warm Sunset",
                                  "Cool Blue", "Vintage Film", "High Contrast", "Soft Light",
                                  "Dramatic",  "Natural"};
    for (const QString& preset : builtInPresets) {
        m_presetCombo->addItem(preset, preset);
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
        QComboBox::down-arrow { image: url(:/assets/icons/dropdown.svg); width: 10px; }
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

    comboLayout->addWidget(m_presetCombo);

    // Save preset button
    ModernButton* saveBtn = new ModernButton("Save", ModernButton::Secondary, this);
    saveBtn->setButtonSize(ModernButton::Small);
    saveBtn->setIcon(":/assets/icons/save.svg");
    saveBtn->setToolTip("Save current adjustments as a new preset");
    // Removed fixed width to allow layout to size it properly
    // Custom styling for extra compactness
    saveBtn->setStyleSheet(R"(
        QPushButton {
            padding: 5px 10px 5px 24px; /* Increased right padding, adjusted left for icon */
            text-align: left;
            font-size: 11px;
            margin: 0px;
        }
    )");
    connect(saveBtn, &QPushButton::clicked, this, &ToolPanel::savePresetButtonClicked);
    comboLayout->addWidget(saveBtn);

    layout->addWidget(container);

    presetsSection->setContentLayout(layout);
    return presetsSection;
}

ModernCollapsible* ToolPanel::createBasicSection() {
    ModernCollapsible* basicSection =
        new ModernCollapsible("Light", ":/assets/icons/light.png", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(8, 8, 8, 12);

    layout->setSpacing(2);
    layout->setContentsMargins(8, 8, 8, 12);

    // Auto buttons row
    QHBoxLayout* autoRow = new QHBoxLayout();
    autoRow->setSpacing(8);

    // Auto Light Button (reset to original, then apply)
    QPushButton* autoBtn = new QPushButton("Auto Light", this);
    autoBtn->setIcon(QIcon(":/assets/icons/auto_fix.png"));
    autoBtn->setToolTip("Reset sliders and auto-adjust from original image");
    autoBtn->setCursor(Qt::PointingHandCursor);
    autoBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            color: #e0e0e0;
            border: 1px solid #505050;
            border-radius: 4px;
            padding: 6px 8px;
            font-weight: bold;
            font-size: 11px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
            border-color: #606060;
        }
        QPushButton:pressed {
            background-color: #2a2a2a;
        }
    )");
    connect(autoBtn, &QPushButton::clicked, this, &ToolPanel::autoLightRequested);
    autoRow->addWidget(autoBtn);

    // Smart Enhance Button (refine current)
    QPushButton* smartBtn = new QPushButton("Enhance", this);
    smartBtn->setIcon(QIcon(":/assets/icons/auto_fix.png"));
    smartBtn->setToolTip("Refine current adjustments (analyze current image)");
    smartBtn->setCursor(Qt::PointingHandCursor);
    smartBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            color: #e0e0e0;
            border: 1px solid #505050;
            border-radius: 4px;
            padding: 6px 8px;
            font-weight: bold;
            font-size: 11px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
            border-color: #606060;
        }
        QPushButton:pressed {
            background-color: #2a2a2a;
        }
    )");
    connect(smartBtn, &QPushButton::clicked, this, &ToolPanel::smartEnhanceRequested);
    autoRow->addWidget(smartBtn);

    layout->addLayout(autoRow);

    // Separator
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #444; margin: 4px 0;");
    layout->addWidget(line);

    // Create modern sliders
    m_exposureSlider = new ModernSlider("Exposure", -50, 50, 0, this);
    m_exposureSlider->setTooltip("Adjust exposure in EV (-5 to +5)\nDouble-click to reset");
    m_exposureSlider->setUnit(" EV");
    m_exposureSlider->setDisplayDivisor(10.0f);    // Display -5 to +5 instead of -50 to +50
    m_exposureSlider->setShowTickMarks(true, 11);  // -5 to +5 = 11 ticks

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

    m_brightnessSlider = new ModernSlider("Brightness", -100, 100, 0, this);
    m_brightnessSlider->setTooltip("Adjust overall brightness\nDouble-click to reset");

    // Connect slider released signals for history
    auto connectHistory = [this](ModernSlider* slider, const QString& name) {
        connect(slider, &ModernSlider::sliderPressed, this, &ToolPanel::adjustmentStarted);
        connect(slider, &ModernSlider::sliderReleased, this,
                [this, name, slider]() { Q_EMIT adjustmentFinished(name, slider->value()); });
    };

    connectHistory(m_exposureSlider, "Exposure");
    connectHistory(m_contrastSlider, "Contrast");
    connectHistory(m_highlightsSlider, "Highlights");
    connectHistory(m_shadowsSlider, "Shadows");
    connectHistory(m_whitesSlider, "Whites");
    connectHistory(m_blacksSlider, "Blacks");
    connectHistory(m_brightnessSlider, "Brightness");

    // Connect signals
    connect(m_exposureSlider, &ModernSlider::valueChanged, this, &ToolPanel::exposureChanged);
    connect(m_contrastSlider, &ModernSlider::valueChanged, this, &ToolPanel::contrastChanged);
    connect(m_highlightsSlider, &ModernSlider::valueChanged, this, &ToolPanel::highlightsChanged);
    connect(m_shadowsSlider, &ModernSlider::valueChanged, this, &ToolPanel::shadowsChanged);
    connect(m_whitesSlider, &ModernSlider::valueChanged, this, &ToolPanel::whitesChanged);
    connect(m_blacksSlider, &ModernSlider::valueChanged, this, &ToolPanel::blacksChanged);
    connect(m_brightnessSlider, &ModernSlider::valueChanged, this, &ToolPanel::brightnessChanged);

    // Add to layout - Brightness directly after Exposure per user request
    layout->addWidget(m_exposureSlider);
    layout->addWidget(m_brightnessSlider);
    layout->addWidget(m_contrastSlider);
    layout->addWidget(m_highlightsSlider);
    layout->addWidget(m_shadowsSlider);
    layout->addWidget(m_whitesSlider);
    layout->addWidget(m_blacksSlider);

    basicSection->setContentLayout(layout);
    return basicSection;
}

ModernCollapsible* ToolPanel::createColorSection() {
    ModernCollapsible* colorSection =
        new ModernCollapsible("Color", ":/assets/icons/color.png", this);
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

    // Connect history
    auto connectHistory = [this](ModernSlider* slider, const QString& name) {
        connect(slider, &ModernSlider::sliderReleased, this,
                [this, name, slider]() { Q_EMIT adjustmentFinished(name, slider->value()); });
    };
    connectHistory(m_temperatureSlider, "Temperature");
    connectHistory(m_tintSlider, "Tint");
    connectHistory(m_saturationSlider, "Saturation");

    // Add to layout
    layout->addWidget(m_temperatureSlider);
    layout->addWidget(m_tintSlider);
    layout->addWidget(m_saturationSlider);

    colorSection->setContentLayout(layout);
    m_colorSection = colorSection;  // Store reference for enabling/disabling
    return colorSection;
}

ModernCollapsible* ToolPanel::createDetailSection() {
    ModernCollapsible* detailSection =
        new ModernCollapsible("Detail", ":/assets/icons/detail.png", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(8, 8, 8, 12);

    // Denoise slider
    m_denoiseSlider = new ModernSlider("Denoise", 0, 100, 0, this);
    m_denoiseSlider->setTooltip(
        "Reduce image noise\nHigher values = stronger denoising\nDouble-click to reset");
    connect(m_denoiseSlider, &ModernSlider::valueChanged, this, &ToolPanel::denoiseChanged);
    layout->addWidget(m_denoiseSlider);

    // Clarity slider
    m_claritySlider = new ModernSlider("Clarity", -100, 100, 0, this);
    m_claritySlider->setTooltip("Enhance midtone contrast\nDouble-click to reset");
    connect(m_claritySlider, &ModernSlider::valueChanged, this, &ToolPanel::clarityChanged);
    layout->addWidget(m_claritySlider);

    // Sharpening slider
    m_sharpeningSlider = new ModernSlider("Sharpening", 0, 100, 0, this);
    m_sharpeningSlider->setTooltip("Sharpen image details\nDouble-click to reset");
    connect(m_sharpeningSlider, &ModernSlider::valueChanged, this, &ToolPanel::sharpeningChanged);
    layout->addWidget(m_sharpeningSlider);

    // Connect history
    auto connectHistory = [this](ModernSlider* slider, const QString& name) {
        connect(slider, &ModernSlider::sliderReleased, this,
                [this, name, slider]() { Q_EMIT adjustmentFinished(name, slider->value()); });
    };
    connectHistory(m_denoiseSlider, "Denoise");
    connectHistory(m_claritySlider, "Clarity");
    connectHistory(m_sharpeningSlider, "Sharpening");

    detailSection->setContentLayout(layout);
    detailSection->setExpanded(false);  // Start collapsed
    return detailSection;
}

ModernCollapsible* ToolPanel::createEffectsSection() {
    ModernCollapsible* effectsSection =
        new ModernCollapsible("Effects", ":/assets/icons/effects.png", this);
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
    ModernCollapsible* styleSection =
        new ModernCollapsible("AI Style Transfer", ":/assets/icons/ai_style.png", this);
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

    // Create variation sliders
    m_styleHueSlider = new ModernSlider("Hue Shift", -100, 100, 0, this);
    m_styleHueSlider->setTooltip("Shift colors around the color wheel\n-100 = Cool, +100 = Warm");
    connect(m_styleHueSlider, &ModernSlider::valueChanged, this, &ToolPanel::styleHueChanged);
    layout->addWidget(m_styleHueSlider);

    m_styleSatSlider = new ModernSlider("Saturation", 0, 100, 0, this);
    m_styleSatSlider->setTooltip("Boost color intensity before style transfer");
    connect(m_styleSatSlider, &ModernSlider::valueChanged, this, &ToolPanel::styleSatChanged);
    layout->addWidget(m_styleSatSlider);

    m_styleContrastSlider = new ModernSlider("Contrast", 0, 100, 0, this);
    m_styleContrastSlider->setTooltip("Adjust contrast before style transfer");
    connect(m_styleContrastSlider, &ModernSlider::valueChanged, this,
            &ToolPanel::styleContrastChanged);
    layout->addWidget(m_styleContrastSlider);

    m_styleNoiseSlider = new ModernSlider("Noise", 0, 100, 0, this);
    m_styleNoiseSlider->setTooltip("Add noise for texture variation");
    connect(m_styleNoiseSlider, &ModernSlider::valueChanged, this, &ToolPanel::styleNoiseChanged);
    layout->addWidget(m_styleNoiseSlider);

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
    m_denoiseSlider->reset();
    m_claritySlider->reset();
    m_claritySlider->reset();
    m_sharpeningSlider->reset();

    m_sharpeningSlider->reset();

    // Reset rotation
    if (m_rotationSlider) {
        m_rotationSlider->reset();
    }

    // Reset transformation UI state
    if (m_cropModeCombo) {
        m_cropModeCombo->setCurrentIndex(0);  // Reset to "Free" or first item
    }

    // Deactivate Straighten Mode if active
    if (m_straightenToggle && m_straightenToggle->isChecked()) {
        m_straightenToggle->setChecked(false);
    }

    // Reset filter gallery selection
    if (m_filterGallery) {
        m_filterGallery->setSelectedFilter("Original");
    }

    // Reset style gallery selection (deselect all)
    if (m_styleGallery) {
        m_styleGallery->clearSelection();
    }

    // Reset style variation sliders
    if (m_styleHueSlider)
        m_styleHueSlider->reset();
    if (m_styleSatSlider)
        m_styleSatSlider->reset();
    if (m_styleContrastSlider)
        m_styleContrastSlider->reset();
    if (m_styleNoiseSlider)
        m_styleNoiseSlider->reset();

    // Notify controller to reset processing parameters
    Q_EMIT resetAllRequested();
}

ModernCollapsible* ToolPanel::createToolSection() {
    ModernCollapsible* toolSection =
        new ModernCollapsible("Transform", ":/assets/icons/transform.png", this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(16);
    layout->setContentsMargins(12, 12, 12, 16);

    // Helper for sectional headers
    auto createHeader = [this](const QString& text) -> QLabel* {
        QLabel* label = new QLabel(text, this);
        label->setStyleSheet(
            "font-weight: 700; color: #808080; font-size: 10px; letter-spacing: 1px; "
            "text-transform: uppercase; margin-bottom: 4px;");
        return label;
    };

    // --- ROTATE & FLIP ---
    // Combined row for efficient space usage
    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(16);

    // Rotate Column
    QVBoxLayout* rotateCol = new QVBoxLayout();
    rotateCol->setSpacing(8);
    rotateCol->addWidget(createHeader("ROTATE"));

    QHBoxLayout* rotateBtns = new QHBoxLayout();
    rotateBtns->setSpacing(8);

    ModernToolButton* rotateLeftBtn =
        new ModernToolButton("Left", ":/assets/icons/rotate_left.png", this);
    rotateLeftBtn->setToolTip("Rotate 90° Left");
    rotateLeftBtn->setFlat(true);
    connect(rotateLeftBtn, &ModernToolButton::clicked, this, &ToolPanel::rotateLeftRequested);

    ModernToolButton* rotateRightBtn =
        new ModernToolButton("Right", ":/assets/icons/rotate_right.png", this);
    rotateRightBtn->setToolTip("Rotate 90° Right");
    rotateRightBtn->setFlat(true);
    connect(rotateRightBtn, &ModernToolButton::clicked, this, &ToolPanel::rotateRightRequested);

    rotateBtns->addWidget(rotateLeftBtn);
    rotateBtns->addWidget(rotateRightBtn);
    rotateCol->addLayout(rotateBtns);
    topRow->addLayout(rotateCol);

    // Flip Column
    QVBoxLayout* flipCol = new QVBoxLayout();
    flipCol->setSpacing(8);
    flipCol->addWidget(createHeader("FLIP"));

    QHBoxLayout* flipBtns = new QHBoxLayout();
    flipBtns->setSpacing(8);

    ModernToolButton* flipHBtn =
        new ModernToolButton("Horiz", ":/assets/icons/flip_horizontal.png", this);
    flipHBtn->setToolTip("Flip Horizontally");
    flipHBtn->setFlat(true);
    connect(flipHBtn, &ModernToolButton::clicked, this, &ToolPanel::flipHorizontalRequested);

    ModernToolButton* flipVBtn =
        new ModernToolButton("Vert", ":/assets/icons/flip_vertical.png", this);
    flipVBtn->setToolTip("Flip Vertically");
    flipVBtn->setFlat(true);
    connect(flipVBtn, &ModernToolButton::clicked, this, &ToolPanel::flipVerticalRequested);

    flipBtns->addWidget(flipHBtn);
    flipBtns->addWidget(flipVBtn);
    flipCol->addLayout(flipBtns);
    topRow->addLayout(flipCol);

    layout->addLayout(topRow);

    // --- MANUAL ROTATION ---
    layout->addWidget(createHeader("FINE ROTATION"));
    m_rotationSlider = new ModernSlider("", -45, 45, 0, this);
    m_rotationSlider->setRange(-180, 180);
    m_rotationSlider->setUnit("°");
    connect(m_rotationSlider, &ModernSlider::valueChanged, this, &ToolPanel::rotateAngleChanged);
    connect(m_rotationSlider, &ModernSlider::sliderReleased, this,
            [this]() { Q_EMIT adjustmentFinished("Rotation", m_rotationSlider->value()); });
    layout->addWidget(m_rotationSlider);

    // --- STRAIGHTEN (Enhanced) ---
    layout->addWidget(createHeader("STRAIGHTEN & CROP"));

    QFrame* straightenFrame = new QFrame(this);
    straightenFrame->setStyleSheet("background: #2a2a2a; border-radius: 6px;");
    QVBoxLayout* straightenLayout = new QVBoxLayout(straightenFrame);
    straightenLayout->setContentsMargins(8, 8, 8, 8);
    straightenLayout->setSpacing(8);

    // Toggle and Apple buttons
    QHBoxLayout* straightRow = new QHBoxLayout();
    m_straightenToggle = new ModernToolButton("Straighten", ":/assets/icons/straighten.png", this);
    m_straightenToggle->setCheckable(true);
    m_straightenToggle->setFlat(true);
    connect(m_straightenToggle, &ModernToolButton::toggled, this,
            &ToolPanel::straightenModeToggled);

    m_applyStraightenBtn = new ModernButton("Apply", ModernButton::Primary, this);
    m_applyStraightenBtn->setEnabled(false);
    connect(m_applyStraightenBtn, &QPushButton::clicked, this, [this]() {
        Q_EMIT applyStraightenRequested();
        if (m_straightenToggle) {
            m_straightenToggle->setChecked(false);  // Auto-exit Straighten Mode
        }
    });
    connect(m_straightenToggle, &ModernToolButton::toggled, m_applyStraightenBtn,
            &QPushButton::setEnabled);

    straightRow->addWidget(m_straightenToggle, 2);
    straightRow->addWidget(m_applyStraightenBtn, 1);
    straightenLayout->addLayout(straightRow);

    // Aspect Ratio Combo
    m_straightenRatioCombo = new QComboBox(this);
    // ... Ratio items ...
    struct RatioItem {
        QString name;
        int id;
    };
    const std::vector<RatioItem> ratios = {
        {"Free", 0},         {"1:1 Square", 1},      {"4:3 Photo", 2},
        {"3:2 Photo", 3},    {"16:9 Widescreen", 4}, {"21:9 Ultrawide", 5},
        {"3:4 Portrait", 6}, {"2:3 Portrait", 7},    {"9:16 Portrait", 8}};
    for (const auto& r : ratios)
        m_straightenRatioCombo->addItem(r.name, r.id);

    QString comboStyle = R"(
        QComboBox { background: #333; border: 1px solid #444; border-radius: 4px; padding: 6px 10px; color: #ddd; min-height: 28px; }
        QComboBox:hover { border-color: #555; background: #383838; }
        QComboBox::focus { border-color: #6366f1; }
        QComboBox::drop-down { border: none; width: 20px; }
        QComboBox QAbstractItemView { background: #2a2a2a; color: #ddd; selection-background-color: #6366f1; border: 1px solid #444; }
    )";
    m_straightenRatioCombo->setStyleSheet(comboStyle);
    m_straightenRatioCombo->setEnabled(false);
    connect(m_straightenToggle, &ModernToolButton::toggled, m_straightenRatioCombo,
            &QPushButton::setEnabled);
    connect(
        m_straightenRatioCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int index) {
            Q_EMIT straightenAspectRatioChanged(m_straightenRatioCombo->itemData(index).toInt());
        });

    straightenLayout->addWidget(m_straightenRatioCombo);
    layout->addWidget(straightenFrame);

    // --- STANDARD OPERATIONS (Crop & Resize) ---
    // Grouping these nicely
    layout->addWidget(createHeader("STANDARD OPERATIONS"));

    // Crop Row
    m_cropModeCombo = new QComboBox(this);
    for (const auto& r : ratios)
        m_cropModeCombo->addItem(r.name, r.id);
    m_cropModeCombo->addItem("Perspective", 99);
    m_cropModeCombo->addItem("Fixed Size...", 100);
    m_cropModeCombo->setStyleSheet(comboStyle);

    QHBoxLayout* cropRow = new QHBoxLayout();
    ModernToolButton* startCropBtn = new ModernToolButton("Crop", ":/assets/icons/crop.png", this);
    startCropBtn->setToolTip("Enter Crop Mode");
    startCropBtn->setFlat(true);
    connect(startCropBtn, &ModernToolButton::clicked, this, &ToolPanel::cropRequested);

    cropRow->addWidget(startCropBtn, 1);
    cropRow->addWidget(m_cropModeCombo, 2);
    layout->addLayout(cropRow);

    // Fixed Size Inputs
    m_fixedSizeWidget = new QWidget(this);
    QHBoxLayout* fsLayout = new QHBoxLayout(m_fixedSizeWidget);
    fsLayout->setContentsMargins(0, 0, 0, 0);
    m_cropWidthSpin = new QSpinBox(this);
    m_cropWidthSpin->setRange(1, 10000);
    m_cropWidthSpin->setValue(800);
    m_cropHeightSpin = new QSpinBox(this);
    m_cropHeightSpin->setRange(1, 10000);
    m_cropHeightSpin->setValue(600);
    QString spinStyle =
        "QSpinBox { background: #333; border: 1px solid #444; padding: 6px; color: #ddd; "
        "min-height: 28px; border-radius: 4px; }";
    m_cropWidthSpin->setStyleSheet(spinStyle);
    m_cropHeightSpin->setStyleSheet(spinStyle);

    fsLayout->addWidget(m_cropWidthSpin);
    fsLayout->addWidget(new QLabel("×", this));
    fsLayout->addWidget(m_cropHeightSpin);
    m_fixedSizeWidget->setVisible(false);
    layout->addWidget(m_fixedSizeWidget);

    connect(m_cropModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                int val = m_cropModeCombo->itemData(index).toInt();
                m_fixedSizeWidget->setVisible(val == 100);
                if (val == 99)
                    Q_EMIT perspectiveCropRequested();
                else if (val == 100)
                    Q_EMIT cropFixedSizeChanged(m_cropWidthSpin->value(),
                                                m_cropHeightSpin->value());
                else
                    Q_EMIT cropAspectRatioChanged(val);
            });
    auto updateFixed = [this](int) {
        if (m_fixedSizeWidget->isVisible())
            Q_EMIT cropFixedSizeChanged(m_cropWidthSpin->value(), m_cropHeightSpin->value());
    };
    connect(m_cropWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, updateFixed);
    connect(m_cropHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, updateFixed);

    // Resize Button
    ModernButton* resizeBtn = new ModernButton("Resize Image...", ModernButton::Secondary, this);
    resizeBtn->setIcon(
        ":/assets/icons/resize.png");  // Assuming icon name? Or keep Emoji if no icon
    resizeBtn->setToolTip("Open resize dialog");
    connect(resizeBtn, &QPushButton::clicked, this, &ToolPanel::showResizeDialog);
    layout->addWidget(resizeBtn);

    toolSection->setContentLayout(layout);
    toolSection->setExpanded(false);
    return toolSection;
}

void ToolPanel::setZoomModeChecked(bool checked) {
    // Block signals to avoid recursive updates
    m_zoomBtn->blockSignals(true);
    m_zoomBtn->setChecked(checked);
    m_zoomBtn->blockSignals(false);
}

void ToolPanel::refreshPresets(const QStringList& userPresets) {
    if (!m_presetCombo)
        return;

    // Block signals during refresh
    m_presetCombo->blockSignals(true);

    // Remember current selection
    QString currentSelection = m_presetCombo->currentData().toString();

    // Clear and rebuild
    m_presetCombo->clear();
    m_presetCombo->addItem("Select a preset...");
    m_presetCombo->insertSeparator(1);

    // Add built-in presets
    QStringList builtInPresets = {"Cinematic", "Portrait",     "Landscape",     "Warm Sunset",
                                  "Cool Blue", "Vintage Film", "High Contrast", "Soft Light",
                                  "Dramatic",  "Natural"};
    for (const QString& preset : builtInPresets) {
        m_presetCombo->addItem(preset, preset);
    }

    // Add separator if there are user presets
    if (!userPresets.isEmpty()) {
        m_presetCombo->insertSeparator(m_presetCombo->count());

        // Add user presets
        for (const QString& preset : userPresets) {
            m_presetCombo->addItem(preset, preset);
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
                              int highlights, int shadows, int whites, int blacks, int temperature,
                              int tint, int denoise, int clarity, int sharpening) {
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
    m_denoiseSlider->setValue(denoise);
    m_claritySlider->setValue(clarity);
    m_sharpeningSlider->setValue(sharpening);
}

void ToolPanel::setRotationSliderValue(int rotation) {
    if (m_rotationSlider) {
        m_rotationSlider->setValue(rotation);
    }
}

void ToolPanel::setColorControlsEnabled(bool enabled) {
    // Enable/disable individual color sliders
    m_temperatureSlider->setEnabled(enabled);
    m_tintSlider->setEnabled(enabled);
    m_saturationSlider->setEnabled(enabled);

    // Apply visual dimming to show disabled state
    // Qt doesn't support CSS opacity, so we use color-based styling
    QString disabledStyle = "QWidget { color: #555; }";
    QString enabledStyle = "";

    m_temperatureSlider->setStyleSheet(enabled ? enabledStyle : disabledStyle);
    m_tintSlider->setStyleSheet(enabled ? enabledStyle : disabledStyle);
    m_saturationSlider->setStyleSheet(enabled ? enabledStyle : disabledStyle);

    qDebug() << "Color controls" << (enabled ? "enabled" : "disabled (grayscale/effect active)");
}

void ToolPanel::updateImageInfo(const QImage& image) {
    if (!image.isNull()) {
        m_currentImageSize = image.size();
    }
}

void ToolPanel::showResizeDialog() {
    if (m_currentImageSize.isValid()) {
        ResizeDialog dialog(m_currentImageSize.width(), m_currentImageSize.height(), this);
        if (dialog.exec() == QDialog::Accepted) {
            int newWidth = dialog.newWidth();
            int newHeight = dialog.newHeight();
            if (newWidth != m_currentImageSize.width() ||
                newHeight != m_currentImageSize.height()) {
                Q_EMIT resizeConfirmed(newWidth, newHeight);
            }
        }
    }
}
