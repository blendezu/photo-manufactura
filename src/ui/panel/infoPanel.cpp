#include "infoPanel.h"

#include <QGroupBox>

InfoPanel::InfoPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

InfoPanel::~InfoPanel() {
    // Qt handles child widget deletion
}

void InfoPanel::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(5);

    // Title
    m_titleLabel = new QLabel(tr("<b>Image Information</b>"), this);
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addSpacing(10);

    // Image metadata group
    QGroupBox* metadataGroup = new QGroupBox(tr("Metadata"), this);
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataGroup);

    m_filePathLabel = new QLabel(tr("File: None"), metadataGroup);
    m_filePathLabel->setWordWrap(true);
    m_dimensionsLabel = new QLabel(tr("Dimensions: N/A"), metadataGroup);
    m_formatLabel = new QLabel(tr("Format: N/A"), metadataGroup);

    metadataLayout->addWidget(m_filePathLabel);
    metadataLayout->addWidget(m_dimensionsLabel);
    metadataLayout->addWidget(m_formatLabel);
    m_mainLayout->addWidget(metadataGroup);

    // Histogram placeholder
    QGroupBox* histogramGroup = new QGroupBox(tr("Histogram"), this);
    QVBoxLayout* histogramLayout = new QVBoxLayout(histogramGroup);
    m_histogramPlaceholder = new QLabel(tr("Histogram will appear here"), histogramGroup);
    m_histogramPlaceholder->setMinimumHeight(150);
    m_histogramPlaceholder->setAlignment(Qt::AlignCenter);
    m_histogramPlaceholder->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    histogramLayout->addWidget(m_histogramPlaceholder);
    m_mainLayout->addWidget(histogramGroup);

    // Add stretch to push everything to the top
    m_mainLayout->addStretch();

    setLayout(m_mainLayout);
}

void InfoPanel::updateImageInfo(const QString& filePath, int width, int height,
                                const QString& format) {
    m_filePathLabel->setText(tr("File: %1").arg(filePath));
    m_dimensionsLabel->setText(tr("Dimensions: %1 x %2").arg(width).arg(height));
    m_formatLabel->setText(tr("Format: %1").arg(format));
}

void InfoPanel::clearInfo() {
    m_filePathLabel->setText(tr("File: None"));
    m_dimensionsLabel->setText(tr("Dimensions: N/A"));
    m_formatLabel->setText(tr("Format: N/A"));
}
