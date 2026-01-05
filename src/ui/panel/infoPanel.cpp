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

    m_zoomLabel = new QLabel(tr("Zoom: 100%"), metadataGroup);
    m_zoomLabel->setStyleSheet(labelStyle + " font-weight: 500;");

    metadataLayout->addWidget(m_filePathLabel);
    metadataLayout->addWidget(m_dimensionsLabel);
    metadataLayout->addWidget(m_megapixelsLabel);
    metadataLayout->addWidget(m_formatLabel);
    metadataLayout->addWidget(m_fileSizeLabel);
    metadataLayout->addWidget(m_colorSpaceLabel);
    metadataLayout->addWidget(m_zoomLabel);
    m_mainLayout->addWidget(metadataGroup);

    // History group with matching styling
    QGroupBox* historyGroup = new QGroupBox(tr("History"), this);
    historyGroup->setStyleSheet(
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
    QVBoxLayout* historyLayout = new QVBoxLayout(historyGroup);
    historyLayout->setContentsMargins(5, 10, 5, 5);

    m_historyList = new QListWidget(historyGroup);
    m_historyList->setFocusPolicy(Qt::NoFocus);  // Don't steal focus from canvas
    m_historyList->setStyleSheet(
        "QListWidget { "
        "  background-color: #2b2b2b; "
        "  border: none; "
        "  color: #ccc; "
        "  font-size: 11px; "
        "  outline: none; "
        "} "
        "QListWidget::item { "
        "  padding: 4px; "
        "  border-bottom: 1px solid #333; "
        "} "
        "QListWidget::item:selected { "
        "  background-color: #3a3a3a; "
        "}");
    // Show only last 5 items roughly
    m_historyList->setFixedHeight(150);

    historyLayout->addWidget(m_historyList);
    m_mainLayout->addWidget(historyGroup);

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

void InfoPanel::updateZoomLevel(double level) {
    int percentage = static_cast<int>(level * 100);
    m_zoomLabel->setText(tr("Zoom: %1%").arg(percentage));
}

void InfoPanel::clearInfo() {
    m_filePathLabel->setText(tr("File: None"));
    m_dimensionsLabel->setText(tr("Dimensions: N/A"));
    m_megapixelsLabel->setText(tr("Megapixels: N/A"));
    m_formatLabel->setText(tr("Format: N/A"));
    m_fileSizeLabel->setText(tr("File Size: N/A"));
    m_colorSpaceLabel->setText(tr("Color Space: N/A"));
    m_zoomLabel->setText(tr("Zoom: 100%"));
    m_histogramWidget->clear();
    m_historyList->clear();
}

void InfoPanel::updateHistory(const QStringList& history) {
    m_historyList->clear();

    if (history.isEmpty()) {
        m_historyList->addItem(tr("Original Image"));
        return;
    }

    // Add items (history list is most recent first)
    // We want to show them top-to-bottom as [Most Recent] ... [Oldest]
    for (const QString& action : history) {
        QListWidgetItem* item = new QListWidgetItem(action);
        // Style the most recent action differently
        if (m_historyList->count() == 0) {
            item->setForeground(QColor("#ffffff"));
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            item->setIcon(QIcon::fromTheme("edit-undo"));  // Optional icon
        }
        m_historyList->addItem(item);
    }

    // Add "Original" at the bottom
    m_historyList->addItem(tr("Original Image"));
}
