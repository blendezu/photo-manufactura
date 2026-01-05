#include "modernToolButton.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

// ============================================================================
// ModernToolButton Implementation
// ============================================================================

ModernToolButton::ModernToolButton(const QString& text, const QString& iconPath, QWidget* parent)
    : QWidget(parent), m_iconPath(iconPath) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(4, 6, 4, 6);
    m_layout->setSpacing(4);
    m_layout->setAlignment(Qt::AlignCenter);

    // Icon button
    m_button = new QPushButton(this);
    m_button->setFixedSize(44, 44);
    m_button->setIconSize(QSize(24, 24));
    m_button->setFocusPolicy(Qt::NoFocus);
    if (!iconPath.isEmpty()) {
        m_button->setIcon(QIcon(iconPath));
    }
    m_button->setAttribute(Qt::WA_TransparentForMouseEvents);  // Pass clicks to parent

    // Label below icon
    m_label = new QLabel(text, this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setStyleSheet("color: #aaa; font-size: 10px;");
    m_label->setAttribute(Qt::WA_TransparentForMouseEvents);

    // Badge (optional, for "AI" tag etc)
    m_badge = new QLabel(this);
    m_badge->setAlignment(Qt::AlignCenter);
    m_badge->setFixedSize(24, 14);
    m_badge->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6366f1, stop:1 #8b5cf6);"
        "border-radius: 7px; color: white; font-size: 8px; font-weight: bold;");
    m_badge->hide();

    m_layout->addWidget(m_button, 0, Qt::AlignCenter);
    m_layout->addWidget(m_label, 0, Qt::AlignCenter);

    setFixedSize(60, 72);
    setCursor(Qt::PointingHandCursor);

    // Shadow effect for depth
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(8);
    m_shadowEffect->setColor(QColor(0, 0, 0, 40));
    m_shadowEffect->setOffset(0, 2);
    m_button->setGraphicsEffect(m_shadowEffect);

    updateStyle();
}

void ModernToolButton::setIcon(const QString& iconPath) {
    m_iconPath = iconPath;
    m_button->setIcon(QIcon(iconPath));
}

void ModernToolButton::setText(const QString& text) {
    m_label->setText(text);
}

void ModernToolButton::setChecked(bool checked) {
    if (m_checked != checked) {
        m_checked = checked;
        updateStyle();
        Q_EMIT toggled(checked);
    }
}

void ModernToolButton::setBadgeText(const QString& text) {
    if (text.isEmpty()) {
        m_badge->hide();
    } else {
        m_badge->setText(text);
        m_badge->show();
        // Position badge at top-right of button
        m_badge->move(m_button->x() + m_button->width() - 20, m_button->y() - 4);
    }
}

void ModernToolButton::enterEvent(QEnterEvent* event) {
    m_hovered = true;
    updateStyle();
    QWidget::enterEvent(event);
}

void ModernToolButton::leaveEvent(QEvent* event) {
    m_hovered = false;
    updateStyle();
    QWidget::leaveEvent(event);
}

void ModernToolButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        updateStyle();
    }
    QWidget::mousePressEvent(event);
}

void ModernToolButton::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        if (m_checkable) {
            setChecked(!m_checked);
        }
        Q_EMIT clicked();
        updateStyle();
    }
    QWidget::mouseReleaseEvent(event);
}

void ModernToolButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    // Background painting handled by stylesheet
}

void ModernToolButton::setFlat(bool flat) {
    if (m_flat != flat) {
        m_flat = flat;
        updateStyle();
    }
}

void ModernToolButton::updateStyle() {
    QString bgColor, borderColor, labelColor;

    if (m_checked) {
        bgColor = "#0078d4";
        borderColor = "#0078d4";
        labelColor = "#fff";
    } else if (m_pressed) {
        bgColor = m_flat ? "rgba(255, 255, 255, 0.12)" : "#2a2a2a";
        borderColor = "#0078d4";
        labelColor = "#ddd";
    } else if (m_hovered) {
        bgColor = m_flat ? "rgba(255, 255, 255, 0.08)" : "#4a4a4a";
        borderColor = m_flat ? "transparent" : "#666";
        labelColor = "#fff";
    } else {
        bgColor = m_flat ? "transparent" : "#3a3a3a";
        borderColor = m_flat ? "transparent" : "#505050";
        labelColor = "#aaa";
    }

    m_button->setStyleSheet(QString("QPushButton {"
                                    "  background-color: %1;"
                                    "  border: 1px solid %2;"
                                    "  border-radius: 8px;"
                                    "}"
                                    "QPushButton:hover { background-color: %1; }")
                                .arg(bgColor, borderColor));

    m_label->setStyleSheet(QString("color: %1; font-size: 10px;").arg(labelColor));

    if (m_hovered) {
        m_shadowEffect->setBlurRadius(12);
        m_shadowEffect->setColor(QColor(0, 120, 212, 60));
    } else {
        m_shadowEffect->setBlurRadius(8);
        m_shadowEffect->setColor(QColor(0, 0, 0, 40));
    }
}

// ============================================================================
// FilterPreviewCard Implementation
// ============================================================================

FilterPreviewCard::FilterPreviewCard(const QString& name, const QString& previewPath,
                                     QWidget* parent)
    : QWidget(parent), m_name(name) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    // Preview image
    m_previewLabel = new QLabel(this);
    m_previewLabel->setFixedSize(64, 64);
    m_previewLabel->setScaledContents(true);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet(
        "background: #2a2a2a; border-radius: 6px; border: 1px solid #404040;");

    if (!previewPath.isEmpty()) {
        QPixmap preview(previewPath);
        if (!preview.isNull()) {
            m_previewLabel->setPixmap(
                preview.scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    }

    // Filter name label
    m_nameLabel = new QLabel(name, this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("color: #aaa; font-size: 9px;");
    m_nameLabel->setWordWrap(true);

    layout->addWidget(m_previewLabel, 0, Qt::AlignCenter);
    layout->addWidget(m_nameLabel, 0, Qt::AlignCenter);

    setFixedSize(76, 92);
    setCursor(Qt::PointingHandCursor);

    updateStyle();
}

void FilterPreviewCard::setPreviewImage(const QImage& preview) {
    if (!preview.isNull()) {
        m_previewLabel->setPixmap(QPixmap::fromImage(preview).scaled(
            64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

void FilterPreviewCard::setSelected(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
        updateStyle();
    }
}

void FilterPreviewCard::enterEvent(QEnterEvent* event) {
    m_hovered = true;
    updateStyle();
    QWidget::enterEvent(event);
}

void FilterPreviewCard::leaveEvent(QEvent* event) {
    m_hovered = false;
    updateStyle();
    QWidget::leaveEvent(event);
}

void FilterPreviewCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        Q_EMIT clicked(m_name);
    }
    QWidget::mousePressEvent(event);
}

void FilterPreviewCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor, borderColor;
    if (m_selected) {
        bgColor = QColor(0, 120, 212, 30);
        borderColor = QColor(0, 120, 212);
    } else if (m_hovered) {
        bgColor = QColor(255, 255, 255, 10);
        borderColor = QColor(100, 100, 100);
    } else {
        bgColor = Qt::transparent;
        borderColor = Qt::transparent;
    }

    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

    painter.fillPath(path, bgColor);
    if (borderColor != Qt::transparent) {
        painter.setPen(QPen(borderColor, 1.5));
        painter.drawPath(path);
    }
}

void FilterPreviewCard::updateStyle() {
    QString labelColor = m_selected ? "#fff" : (m_hovered ? "#ddd" : "#aaa");
    m_nameLabel->setStyleSheet(QString("color: %1; font-size: 9px;").arg(labelColor));

    QString previewBorder = m_selected ? "#0078d4" : (m_hovered ? "#555" : "#404040");
    m_previewLabel->setStyleSheet(
        QString("background: #2a2a2a; border-radius: 6px; border: 2px solid %1;")
            .arg(previewBorder));

    update();
}

// ============================================================================
// FilterGalleryWidget Implementation
// ============================================================================

FilterGalleryWidget::FilterGalleryWidget(QWidget* parent) : QWidget(parent) {
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
    m_layout->addStretch();
}

void FilterGalleryWidget::addFilter(const QString& name, const QString& previewPath) {
    FilterPreviewCard* card = new FilterPreviewCard(name, previewPath, this);
    connect(card, &FilterPreviewCard::clicked, this, [this](const QString& filterName) {
        setSelectedFilter(filterName);
        Q_EMIT filterSelected(filterName);
    });

    // Insert before stretch
    m_layout->insertWidget(m_layout->count() - 1, card);
    m_cards.append(card);
}

void FilterGalleryWidget::clearFilters() {
    for (auto* card : m_cards) {
        m_layout->removeWidget(card);
        card->deleteLater();
    }
    m_cards.clear();
    m_selectedFilter.clear();
}

void FilterGalleryWidget::setSelectedFilter(const QString& name) {
    m_selectedFilter = name;
    for (auto* card : m_cards) {
        card->setSelected(card->filterName() == name);
    }
}
