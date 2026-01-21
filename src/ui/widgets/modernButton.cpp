#include "modernButton.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

// ============================================================================
// ModernButton Implementation
// ============================================================================

ModernButton::ModernButton(const QString& text, Style style, QWidget* parent)
    : QPushButton(text, parent),
      m_style(style),
      m_size(Medium),
      m_loading(false),
      m_hoverProgress(0.0) {
    m_hoverAnimation = new QPropertyAnimation(this, "hoverProgress", this);
    m_hoverAnimation->setDuration(150);

    setCursor(Qt::PointingHandCursor);
    updateStyle();
}

void ModernButton::setButtonStyle(Style style) {
    m_style = style;
    updateStyle();
}

void ModernButton::setButtonSize(Size size) {
    m_size = size;
    updateStyle();
}

void ModernButton::setIcon(const QString& iconPath) {
    QPushButton::setIcon(QIcon(iconPath));
    setIconSize(QSize(16, 16));
}

void ModernButton::setIconEmoji(const QString& emoji) {
    m_emoji = emoji;
    update();
}

void ModernButton::setLoading(bool loading) {
    m_loading = loading;
    setEnabled(!loading);
    update();
}

void ModernButton::setHoverProgress(qreal progress) {
    m_hoverProgress = progress;
    update();
}

void ModernButton::enterEvent(QEnterEvent* event) {
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(m_hoverProgress);
    m_hoverAnimation->setEndValue(1.0);
    m_hoverAnimation->start();
    QPushButton::enterEvent(event);
}

void ModernButton::leaveEvent(QEvent* event) {
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(m_hoverProgress);
    m_hoverAnimation->setEndValue(0.0);
    m_hoverAnimation->start();
    QPushButton::leaveEvent(event);
}

void ModernButton::updateStyle() {
    // Size settings
    int hPadding, vPadding, fontSize, minHeight;
    switch (m_size) {
        case Small:
            hPadding = 10;
            vPadding = 4;
            fontSize = 11;
            minHeight = 26;
            break;
        case Large:
            hPadding = 24;
            vPadding = 12;
            fontSize = 14;
            minHeight = 44;
            break;
        default:  // Medium
            hPadding = 16;
            vPadding = 8;
            fontSize = 12;
            minHeight = 34;
            break;
    }

    // Color settings
    QString bgNormal, bgHover, bgPressed, textColor, borderColor;
    switch (m_style) {
        case Primary:
            bgNormal = "#0078d4";
            bgHover = "#1084d8";
            bgPressed = "#006cbd";
            textColor = "#ffffff";
            borderColor = "transparent";
            break;
        case Ghost:
            bgNormal = "transparent";
            bgHover = "rgba(255, 255, 255, 0.08)";
            bgPressed = "rgba(255, 255, 255, 0.12)";
            textColor = "#e0e0e0";
            borderColor = "#505050";
            break;
        case Danger:
            bgNormal = "#d83b01";
            bgHover = "#e84c15";
            bgPressed = "#c43200";
            textColor = "#ffffff";
            borderColor = "transparent";
            break;
        case Success:
            bgNormal = "#107c10";
            bgHover = "#1e9e1e";
            bgPressed = "#0d6b0d";
            textColor = "#ffffff";
            borderColor = "transparent";
            break;
        default:  // Secondary
            bgNormal = "#3a3a3a";
            bgHover = "#454545";
            bgPressed = "#2d2d2d";
            textColor = "#e0e0e0";
            borderColor = "#505050";
            break;
    }

    QString style = QString(R"(
        QPushButton {
            background: %1;
            color: %4;
            border: 1px solid %5;
            border-radius: 6px;
            padding: %3px %2px;
            font-size: %6px;
            font-weight: 500;
            min-height: %7px;
        }
        QPushButton:hover {
            background: %8;
        }
        QPushButton:pressed {
            background: %9;
        }
        QPushButton:disabled {
            background: #2d2d2d;
            color: #666;
            border-color: #3a3a3a;
        }
    )")
                        .arg(bgNormal)
                        .arg(hPadding)
                        .arg(vPadding)
                        .arg(textColor)
                        .arg(borderColor)
                        .arg(fontSize)
                        .arg(minHeight)
                        .arg(bgHover)
                        .arg(bgPressed);

    setStyleSheet(style);
}

void ModernButton::paintEvent(QPaintEvent* event) {
    QPushButton::paintEvent(event);

    if (!m_emoji.isEmpty()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);

        // Draw emoji on the left
        QRect emojiRect(8, 0, 20, height());
        painter.drawText(emojiRect, Qt::AlignCenter, m_emoji);
    }
}

// ============================================================================
// IconButton Implementation
// ============================================================================

IconButton::IconButton(const QString& iconPath, const QString& tooltip, QWidget* parent)
    : QPushButton(parent), m_active(false) {
    if (!iconPath.isEmpty()) {
        setIcon(QIcon(iconPath));
        setIconSize(QSize(18, 18));
    }
    if (!tooltip.isEmpty()) {
        setToolTip(tooltip);
    }

    setFixedSize(32, 32);
    setCursor(Qt::PointingHandCursor);
    updateStyle(false);
}

void IconButton::setIconEmoji(const QString& emoji) {
    setText(emoji);
    setIcon(QIcon());
}

void IconButton::setActive(bool active) {
    m_active = active;
    updateStyle(false);
}

void IconButton::updateStyle(bool hovered) {
    QString bgColor;
    QString borderColor;

    if (m_active) {
        bgColor = hovered ? "#0088e4" : "#0078d4";
        borderColor = "#0078d4";
    } else {
        bgColor = hovered ? "#404040" : "transparent";
        borderColor = hovered ? "#505050" : "transparent";
    }

    setStyleSheet(QString(R"(
        QPushButton {
            background: %1;
            border: 1px solid %2;
            border-radius: 6px;
            color: %3;
            font-size: 14px;
        }
    )")
                      .arg(bgColor, borderColor, m_active ? "#ffffff" : "#c0c0c0"));
}

void IconButton::enterEvent(QEnterEvent* event) {
    updateStyle(true);
    QPushButton::enterEvent(event);
}

void IconButton::leaveEvent(QEvent* event) {
    updateStyle(false);
    QPushButton::leaveEvent(event);
}

// ============================================================================
// ToggleButton Implementation
// ============================================================================

ToggleButton::ToggleButton(const QString& labelOff, const QString& labelOn, QWidget* parent)
    : QWidget(parent),
      m_labelOff(labelOff),
      m_labelOn(labelOn),
      m_checked(false),
      m_animationProgress(0.0) {
    setFixedSize(60, 28);
    setCursor(Qt::PointingHandCursor);

    m_animation = new QPropertyAnimation(this, "");
    m_animation->setDuration(150);
}

void ToggleButton::setChecked(bool checked) {
    if (m_checked == checked)
        return;
    m_checked = checked;
    m_animationProgress = checked ? 1.0 : 0.0;
    update();
    Q_EMIT toggled(checked);
}

void ToggleButton::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    setChecked(!m_checked);
}

void ToggleButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background track
    QColor trackColor = m_checked ? QColor("#0078d4") : QColor("#404040");
    painter.setBrush(trackColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), height() / 2, height() / 2);

    // Handle
    int handleSize = height() - 6;
    int handleX = m_checked ? (width() - handleSize - 3) : 3;
    int handleY = 3;

    // Handle shadow
    painter.setBrush(QColor(0, 0, 0, 40));
    painter.drawEllipse(handleX + 1, handleY + 1, handleSize, handleSize);

    // Handle
    QLinearGradient handleGrad(0, handleY, 0, handleY + handleSize);
    handleGrad.setColorAt(0, QColor("#ffffff"));
    handleGrad.setColorAt(1, QColor("#e8e8e8"));
    painter.setBrush(handleGrad);
    painter.setPen(QPen(QColor("#c0c0c0"), 1));
    painter.drawEllipse(handleX, handleY, handleSize, handleSize);
}
