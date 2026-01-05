#include "modernCollapsible.h"

#include <QEasingCurve>
#include <QEnterEvent>

ModernCollapsible::ModernCollapsible(const QString& title, const QString& icon, QWidget* parent)
    : QWidget(parent),
      m_title(title),
      m_iconText(icon),
      m_expanded(true),
      m_currentContentHeight(0),
      m_targetContentHeight(0),
      m_animationDuration(200) {
    setupUI();

    m_animation = new QPropertyAnimation(this, "contentHeight", this);
    m_animation->setDuration(m_animationDuration);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void ModernCollapsible::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header widget
    m_headerWidget = new QWidget(this);
    m_headerWidget->setCursor(Qt::PointingHandCursor);
    m_headerWidget->setFixedHeight(36);

    QHBoxLayout* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(12, 0, 12, 0);
    headerLayout->setSpacing(8);

    // Icon (emoji or symbol or image path)
    m_iconLabel = new QLabel(this);  // Don't set text immediately
    m_iconLabel->setStyleSheet("font-size: 14px;");
    if (!m_iconText.isEmpty()) {
        setIcon(m_iconText);  // Use setIcon to handle logic
        headerLayout->addWidget(m_iconLabel);
    }
    // ...

    // Title
    m_titleLabel = new QLabel(m_title, this);
    m_titleLabel->setStyleSheet(R"(
        QLabel {
            color: #e0e0e0;
            font-size: 12px;
            font-weight: 600;
            letter-spacing: 0.5px;
        }
    )");
    headerLayout->addWidget(m_titleLabel);

    // Badge (optional)
    m_badgeLabel = new QLabel(this);
    m_badgeLabel->setStyleSheet(R"(
        QLabel {
            background: #0078d4;
            color: white;
            font-size: 9px;
            font-weight: bold;
            padding: 2px 6px;
            border-radius: 8px;
        }
    )");
    m_badgeLabel->hide();
    headerLayout->addWidget(m_badgeLabel);

    headerLayout->addStretch();

    // Chevron
    m_chevronLabel = new QLabel("▼", this);
    m_chevronLabel->setStyleSheet(R"(
        QLabel {
            color: #666;
            font-size: 10px;
        }
    )");
    headerLayout->addWidget(m_chevronLabel);

    updateHeaderStyle(false);

    // Content container
    m_contentContainer = new QWidget(this);
    m_contentContainer->setStyleSheet("background: transparent;");
    m_contentLayout = new QVBoxLayout(m_contentContainer);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    mainLayout->addWidget(m_headerWidget);
    mainLayout->addWidget(m_contentContainer);

    // Connect header click
    m_headerWidget->installEventFilter(this);
    connect(m_headerWidget, &QWidget::destroyed, []() {});

    // Make header clickable
    m_headerWidget->setMouseTracking(true);
}

void ModernCollapsible::setContentLayout(QLayout* contentLayout) {
    // Clear existing content
    QLayoutItem* child;
    while ((child = m_contentLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->setParent(nullptr);
        }
        delete child;
    }

    // Create content widget
    QWidget* contentWidget = new QWidget();
    contentWidget->setLayout(contentLayout);
    m_contentLayout->addWidget(contentWidget);

    // Calculate target height
    m_targetContentHeight = contentWidget->sizeHint().height();

    // Set initial height based on expanded state
    if (m_expanded) {
        m_currentContentHeight = m_targetContentHeight;
        m_contentContainer->setFixedHeight(m_targetContentHeight);
    } else {
        m_currentContentHeight = 0;
        m_contentContainer->setFixedHeight(0);
    }

    updateChevron();
}

void ModernCollapsible::setContentHeight(int height) {
    m_currentContentHeight = height;
    m_contentContainer->setFixedHeight(height);
}

void ModernCollapsible::toggle() {
    setExpanded(!m_expanded);
}

void ModernCollapsible::setExpanded(bool expanded) {
    if (m_expanded == expanded)
        return;

    m_expanded = expanded;
    updateChevron();

    m_animation->stop();
    m_animation->setStartValue(m_currentContentHeight);
    m_animation->setEndValue(expanded ? m_targetContentHeight : 0);
    m_animation->start();

    Q_EMIT expandedChanged(expanded);
}

void ModernCollapsible::setBadgeCount(int count) {
    if (count > 0) {
        m_badgeLabel->setText(QString::number(count));
        m_badgeLabel->show();
    } else {
        m_badgeLabel->hide();
    }
}

void ModernCollapsible::setIcon(const QString& icon) {
    m_iconText = icon;
    if (icon.isEmpty()) {
        m_iconLabel->hide();
        return;
    }

    if (icon.startsWith(":/") || icon.endsWith(".png") || icon.endsWith(".svg")) {
        QPixmap pixmap(icon);
        if (!pixmap.isNull()) {
            m_iconLabel->setPixmap(
                pixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_iconLabel->setText("");
        } else {
            m_iconLabel->setText(icon);
        }
    } else {
        m_iconLabel->setPixmap(QPixmap());
        m_iconLabel->setText(icon);
    }
    m_iconLabel->setVisible(true);
}

void ModernCollapsible::updateHeaderStyle(bool hovered) {
    QString bgColor = hovered ? "#353535" : "#2a2a2a";
    QString borderColor = hovered ? "#454545" : "#383838";

    m_headerWidget->setStyleSheet(QString(R"(
        QWidget {
            background: %1;
            border-bottom: 1px solid %2;
            border-radius: 4px 4px 0 0;
        }
    )")
                                      .arg(bgColor, borderColor));

    if (hovered) {
        m_titleLabel->setStyleSheet(R"(
            QLabel {
                color: #ffffff;
                font-size: 12px;
                font-weight: 600;
                letter-spacing: 0.5px;
            }
        )");
        m_chevronLabel->setStyleSheet(R"(
            QLabel {
                color: #999;
                font-size: 10px;
            }
        )");
    } else {
        m_titleLabel->setStyleSheet(R"(
            QLabel {
                color: #e0e0e0;
                font-size: 12px;
                font-weight: 600;
                letter-spacing: 0.5px;
            }
        )");
        m_chevronLabel->setStyleSheet(R"(
            QLabel {
                color: #666;
                font-size: 10px;
            }
        )");
    }
}

void ModernCollapsible::updateChevron() {
    m_chevronLabel->setText(m_expanded ? "▼" : "▶");
}

void ModernCollapsible::enterEvent(QEnterEvent* event) {
    Q_UNUSED(event);
    updateHeaderStyle(true);
}

void ModernCollapsible::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    updateHeaderStyle(false);
}

// Override mouse press on header
bool ModernCollapsible::event(QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (m_headerWidget->geometry().contains(mouseEvent->pos())) {
            toggle();
            return true;
        }
    }
    return QWidget::event(event);
}

// ============================================================================
// SectionHeader Implementation
// ============================================================================

SectionHeader::SectionHeader(const QString& title, const QString& icon, QWidget* parent)
    : QWidget(parent), m_title(title), m_icon(icon) {
    setupUI();
}

void SectionHeader::setupUI() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 12, 8, 4);
    layout->setSpacing(6);

    if (!m_icon.isEmpty()) {
        QLabel* iconLabel = new QLabel(m_icon, this);
        iconLabel->setStyleSheet("font-size: 12px;");
        layout->addWidget(iconLabel);
    }

    QLabel* titleLabel = new QLabel(m_title.toUpper(), this);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #888;
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 1.5px;
        }
    )");
    layout->addWidget(titleLabel);

    // Line
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background: #404040; max-height: 1px;");
    layout->addWidget(line, 1);

    setFixedHeight(32);
}
