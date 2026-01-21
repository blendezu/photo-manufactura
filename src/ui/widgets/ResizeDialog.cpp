#include "ResizeDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

ResizeDialog::ResizeDialog(int currentWidth, int currentHeight, QWidget* parent)
    : QDialog(parent)
    , m_originalWidth(currentWidth)
    , m_originalHeight(currentHeight)
    , m_aspectRatio(static_cast<double>(currentWidth) / currentHeight) {
    setWindowTitle("Resize Image");
    setMinimumWidth(300);
    setupUI();
}

void ResizeDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* titleLabel = new QLabel("Resize Image", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #e0e0e0;");
    mainLayout->addWidget(titleLabel);

    // Current size info
    m_previewLabel = new QLabel(
        QString("Original: %1 × %2 px").arg(m_originalWidth).arg(m_originalHeight), this);
    m_previewLabel->setStyleSheet("color: #888; font-size: 11px;");
    mainLayout->addWidget(m_previewLabel);

    // Width input
    QHBoxLayout* widthLayout = new QHBoxLayout();
    QLabel* widthLabel = new QLabel("Width:", this);
    widthLabel->setFixedWidth(60);
    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(1, 10000);
    m_widthSpin->setValue(m_originalWidth);
    m_widthSpin->setSuffix(" px");
    m_widthSpin->setStyleSheet(R"(
        QSpinBox {
            background: #2a2a2a;
            border: 1px solid #3a3a3a;
            border-radius: 6px;
            padding: 8px 12px;
            color: #d0d0d0;
            font-size: 13px;
        }
        QSpinBox:hover { border-color: #4a4a4a; }
        QSpinBox:focus { border-color: #6366f1; }
    )");
    widthLayout->addWidget(widthLabel);
    widthLayout->addWidget(m_widthSpin);
    mainLayout->addLayout(widthLayout);

    // Height input
    QHBoxLayout* heightLayout = new QHBoxLayout();
    QLabel* heightLabel = new QLabel("Height:", this);
    heightLabel->setFixedWidth(60);
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(1, 10000);
    m_heightSpin->setValue(m_originalHeight);
    m_heightSpin->setSuffix(" px");
    m_heightSpin->setStyleSheet(m_widthSpin->styleSheet());
    heightLayout->addWidget(heightLabel);
    heightLayout->addWidget(m_heightSpin);
    mainLayout->addLayout(heightLayout);

    // Lock aspect ratio checkbox
    m_lockAspectCheck = new QCheckBox("Lock aspect ratio", this);
    m_lockAspectCheck->setChecked(true);
    m_lockAspectCheck->setStyleSheet("color: #d0d0d0; font-size: 12px;");
    mainLayout->addWidget(m_lockAspectCheck);

    // Connect signals
    connect(m_widthSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &ResizeDialog::onWidthChanged);
    connect(m_heightSpin, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &ResizeDialog::onHeightChanged);
    connect(m_lockAspectCheck, &QCheckBox::toggled, 
            this, &ResizeDialog::onLockAspectChanged);

    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->setStyleSheet(R"(
        QPushButton {
            background: #3a3a3a;
            border: 1px solid #4a4a4a;
            border-radius: 6px;
            padding: 8px 20px;
            color: #d0d0d0;
            font-size: 12px;
        }
        QPushButton:hover { background: #4a4a4a; }
        QPushButton:pressed { background: #2a2a2a; }
        QPushButton[default="true"] { background: #6366f1; border-color: #6366f1; }
        QPushButton[default="true"]:hover { background: #5558e0; }
    )");
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Style the dialog
    setStyleSheet("QDialog { background: #1e1e1e; }");
}

int ResizeDialog::newWidth() const {
    return m_widthSpin->value();
}

int ResizeDialog::newHeight() const {
    return m_heightSpin->value();
}

void ResizeDialog::onWidthChanged(int value) {
    if (m_updating) return;
    if (m_lockAspectCheck->isChecked()) {
        m_updating = true;
        int newHeight = static_cast<int>(value / m_aspectRatio);
        m_heightSpin->setValue(newHeight);
        m_updating = false;
    }
    updateFromWidth();
}

void ResizeDialog::onHeightChanged(int value) {
    if (m_updating) return;
    if (m_lockAspectCheck->isChecked()) {
        m_updating = true;
        int newWidth = static_cast<int>(value * m_aspectRatio);
        m_widthSpin->setValue(newWidth);
        m_updating = false;
    }
    updateFromHeight();
}

void ResizeDialog::onLockAspectChanged(bool checked) {
    if (checked) {
        // Recalculate aspect ratio from current values
        m_aspectRatio = static_cast<double>(m_widthSpin->value()) / m_heightSpin->value();
    }
}

void ResizeDialog::updateFromWidth() {
    int w = m_widthSpin->value();
    int h = m_heightSpin->value();
    double scale = static_cast<double>(w) / m_originalWidth * 100;
    m_previewLabel->setText(QString("New: %1 × %2 px (%3%)").arg(w).arg(h).arg(scale, 0, 'f', 1));
}

void ResizeDialog::updateFromHeight() {
    updateFromWidth();  // Same logic
}
