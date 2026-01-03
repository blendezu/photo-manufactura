#include "infoPanel.h"

#include <QGroupBox>

#include "../widgets/histogramWidget.h"

InfoPanel::InfoPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

InfoPanel::~InfoPanel() {
    // Qt handles child widget deletion
}

void InfoPanel::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(8);

    // Title with better styling
    m_titleLabel = new QLabel(tr("<b>Image Information</b>"), this);
    m_titleLabel->setStyleSheet("font-size: 14px; color: #ddd; padding-bottom: 5px;");
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addSpacing(5);

    // Histogram widget (at top for prominence)
    QGroupBox* histogramGroup = new QGroupBox(tr("Histogram"), this);
    histogramGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: 500; "
        "  border: 1px solid #3a3a3a; "
        "  border-radius: 5px; "
        "  margin-top: 10px; "
        "  padding-top: 10px; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  subcontrol-position: top left; "
        "  padding: 0 5px; "
        "  color: #aaa; "
        "}");
    QVBoxLayout* histogramLayout = new QVBoxLayout(histogramGroup);
    histogramLayout->setContentsMargins(5, 10, 5, 5);
    m_histogramWidget = new HistogramWidget(histogramGroup);
    histogramLayout->addWidget(m_histogramWidget);
    m_mainLayout->addWidget(histogramGroup);

    // Image metadata group with better styling
    QGroupBox* metadataGroup = new QGroupBox(tr("Metadata"), this);
    metadataGroup->setStyleSheet(
        "QGroupBox { "
        "  font-weight: 500; "
        "  border: 1px solid #3a3a3a; "
        "  border-radius: 5px; "
        "  margin-top: 10px; "
        "  padding-top: 10px; "
        "} "
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  subcontrol-position: top left; "
        "  padding: 0 5px; "
        "  color: #aaa; "
        "}");
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataGroup);
    metadataLayout->setSpacing(6);

    QString labelStyle = "color: #bbb; padding: 2px;";

    m_filePathLabel = new QLabel(tr("File: None"), metadataGroup);
    m_filePathLabel->setWordWrap(true);
    m_filePathLabel->setStyleSheet(labelStyle);

    m_dimensionsLabel = new QLabel(tr("Dimensions: N/A"), metadataGroup);
    m_dimensionsLabel->setStyleSheet(labelStyle);

    m_megapixelsLabel = new QLabel(tr("Megapixels: N/A"), metadataGroup);
    m_megapixelsLabel->setStyleSheet(labelStyle);

    m_formatLabel = new QLabel(tr("Format: N/A"), metadataGroup);
    m_formatLabel->setStyleSheet(labelStyle);

    m_fileSizeLabel = new QLabel(tr("File Size: N/A"), metadataGroup);
    m_fileSizeLabel->setStyleSheet(labelStyle);

    m_colorSpaceLabel = new QLabel(tr("Color Space: N/A"), metadataGroup);
    m_colorSpaceLabel->setStyleSheet(labelStyle);

    m_mouseCoordsLabel = new QLabel(tr("Mouse: N/A"), metadataGroup);
    m_mouseCoordsLabel->setStyleSheet(labelStyle);

    metadataLayout->addWidget(m_filePathLabel);
    metadataLayout->addWidget(m_dimensionsLabel);
    metadataLayout->addWidget(m_megapixelsLabel);
    metadataLayout->addWidget(m_formatLabel);
    metadataLayout->addWidget(m_fileSizeLabel);
    metadataLayout->addWidget(m_colorSpaceLabel);
    metadataLayout->addWidget(m_mouseCoordsLabel);
    m_mainLayout->addWidget(metadataGroup);

    // Add stretch to push everything to the top
    m_mainLayout->addStretch();

    setLayout(m_mainLayout);
}

void InfoPanel::updateImageInfo(const QString& filePath, int width, int height,
                                const QString& format) {
    m_filePathLabel->setText(tr("File: %1").arg(filePath));
    m_dimensionsLabel->setText(tr("Dimensions: %1 × %2 px").arg(width).arg(height));

    double megapixels = (width * height) / 1000000.0;
    m_megapixelsLabel->setText(tr("Megapixels: %1 MP").arg(megapixels, 0, 'f', 2));

    m_formatLabel->setText(tr("Format: %1").arg(format));
    m_colorSpaceLabel->setText(tr("Color Space: sRGB"));  // Default assumption
}

void InfoPanel::updateHistogram(const QImage& image) {
    m_histogramWidget->setImage(image);
}

void InfoPanel::updateFileSize(qint64 sizeBytes) {
    QString sizeText;
    if (sizeBytes < 1024) {
        sizeText = tr("%1 bytes").arg(sizeBytes);
    } else if (sizeBytes < 1024 * 1024) {
        sizeText = tr("%1 KB").arg(sizeBytes / 1024.0, 0, 'f', 1);
    } else {
        sizeText = tr("%1 MB").arg(sizeBytes / (1024.0 * 1024.0), 0, 'f', 2);
    }
    m_fileSizeLabel->setText(tr("File Size: %1").arg(sizeText));
}

void InfoPanel::updateMouseCoords(const QPoint& widgetPos, const QPoint& imagePos) {
    m_mouseCoordsLabel->setText(tr("Mouse: Widget(%1,%2) → Image(%3,%4)")
                                    .arg(widgetPos.x())
                                    .arg(widgetPos.y())
                                    .arg(imagePos.x())
                                    .arg(imagePos.y()));
}

void InfoPanel::clearInfo() {
    m_filePathLabel->setText(tr("File: None"));
    m_dimensionsLabel->setText(tr("Dimensions: N/A"));
    m_megapixelsLabel->setText(tr("Megapixels: N/A"));
    m_formatLabel->setText(tr("Format: N/A"));
    m_fileSizeLabel->setText(tr("File Size: N/A"));
    m_colorSpaceLabel->setText(tr("Color Space: N/A"));
    m_mouseCoordsLabel->setText(tr("Mouse: N/A"));
    m_histogramWidget->clear();
}
