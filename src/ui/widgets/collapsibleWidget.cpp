#include "collapsibleWidget.h"

#include <QPropertyAnimation>
#include <QVBoxLayout>

CollapsibleWidget::CollapsibleWidget(const QString& title, QWidget* parent) : QWidget(parent) {
    // Initialize UI components
    m_toggleButton = new QToolButton(this);
    m_toggleButton->setStyleSheet("QToolButton { border: none; }");
    m_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggleButton->setArrowType(Qt::ArrowType::RightArrow);
    m_toggleButton->setText(title);
    m_toggleButton->setCheckable(true);  // Allow toggling
    m_toggleButton->setChecked(true);    // Start expanded
    // Header line
    m_headerLine = new QFrame(this);
    m_headerLine->setFrameShape(QFrame::HLine);
    m_headerLine->setFrameShadow(QFrame::Sunken);
    m_headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // Content area
    m_contentArea = new QScrollArea(this);
    m_contentArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");
    m_contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_contentArea->setWidgetResizable(true);
    m_contentArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_contentArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_contentArea->setMaximumHeight(0);  // Start collapsed visually
    m_contentArea->setMinimumHeight(0);
    // Animation setup
    m_animationDuration = 150;
    m_toggleAnimation = new QParallelAnimationGroup(this);
    QPropertyAnimation* toggleAnim = new QPropertyAnimation(m_contentArea, "maximumHeight", this);
    toggleAnim->setDuration(m_animationDuration);
    m_toggleAnimation->addAnimation(toggleAnim);
    // Layout setup
    m_mainLayout = new QGridLayout(this);
    m_mainLayout->setVerticalSpacing(3);
    m_mainLayout->setContentsMargins(0, 3, 0, 5);
    m_mainLayout->addWidget(m_toggleButton, 0, 0, 1, 1, Qt::AlignLeft);
    m_mainLayout->addWidget(m_headerLine, 0, 2, 1, 1);
    m_mainLayout->addWidget(m_contentArea, 1, 0, 1, 3);
    setLayout(m_mainLayout);

    connect(m_toggleButton, &QToolButton::toggled, this, &CollapsibleWidget::toggle);
}

// Set the layout for the collapsible content area
void CollapsibleWidget::setContentLayout(QLayout* contentLayout) {
    // Remove any existing widget
    if (m_contentArea->widget()) {
        delete m_contentArea->widget();
    }

    // Create a container widget for the layout
    QWidget* contentWidget = new QWidget();
    contentWidget->setLayout(contentLayout);
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Set the widget to the scroll area
    m_contentArea->setWidget(contentWidget);

    // Calculate heights for animation
    const auto collapsedHeight = sizeHint().height() - m_contentArea->maximumHeight();
    auto contentHeight = contentWidget->sizeHint().height();

    // Update animation
    for (int i = 0; i < m_toggleAnimation->animationCount() - 1; ++i) {
        QPropertyAnimation* anim =
            static_cast<QPropertyAnimation*>(m_toggleAnimation->animationAt(i));
        anim->setStartValue(collapsedHeight);
        anim->setEndValue(collapsedHeight + contentHeight);
    }
    // Update content area animation
    QPropertyAnimation* contentAnimation = static_cast<QPropertyAnimation*>(
        m_toggleAnimation->animationAt(m_toggleAnimation->animationCount() - 1));
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);

    // Store content height for instant toggle
    m_contentHeight = contentHeight;

    // If checked (expanded), set the height immediately
    if (m_toggleButton->isChecked()) {
        m_contentArea->setMaximumHeight(contentHeight);
        m_toggleButton->setArrowType(Qt::ArrowType::DownArrow);
    }
}

// Toggle the collapsible section
void CollapsibleWidget::toggle(bool collapsed) {
    // collapsed=true means button is checked (expanded state)
    // collapsed=false means button is unchecked (collapsed state)
    m_toggleButton->setArrowType(collapsed ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    m_toggleAnimation->setDirection(collapsed ? QAbstractAnimation::Forward
                                              : QAbstractAnimation::Backward);
    m_toggleAnimation->start();
}
