#include "collapsibleWidget.h"

#include <QPropertyAnimation>
#include <QVBoxLayout>

CollapsibleWidget::CollapsibleWidget(const QString& title, QWidget* parent) : QWidget(parent) {
    m_toggleButton = new QToolButton(this);
    m_toggleButton->setStyleSheet("QToolButton { border: none; }");
    m_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggleButton->setArrowType(Qt::ArrowType::RightArrow);
    m_toggleButton->setText(title);
    m_toggleButton->setCheckable(true);
    m_toggleButton->setChecked(false);

    m_headerLine = new QFrame(this);
    m_headerLine->setFrameShape(QFrame::HLine);
    m_headerLine->setFrameShadow(QFrame::Sunken);
    m_headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    m_contentArea = new QScrollArea(this);
    m_contentArea->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");
    m_contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_contentArea->setMaximumHeight(0);
    m_contentArea->setMinimumHeight(0);

    m_toggleAnimation = new QParallelAnimationGroup(this);
    QPropertyAnimation* toggleAnim = new QPropertyAnimation(m_contentArea, "maximumHeight", this);
    toggleAnim->setDuration(m_animationDuration);
    m_toggleAnimation->addAnimation(toggleAnim);

    m_mainLayout = new QGridLayout(this);
    m_mainLayout->setVerticalSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_toggleButton, 0, 0, 1, 1, Qt::AlignLeft);
    m_mainLayout->addWidget(m_headerLine, 0, 2, 1, 1);
    m_mainLayout->addWidget(m_contentArea, 1, 0, 1, 3);
    setLayout(m_mainLayout);

    connect(m_toggleButton, &QToolButton::toggled, this, &CollapsibleWidget::toggle);
}

void CollapsibleWidget::setContentLayout(QLayout* contentLayout) {
    delete m_contentArea->layout();
    m_contentArea->setLayout(contentLayout);
    const auto collapsedHeight = sizeHint().height() - m_contentArea->maximumHeight();
    auto contentHeight = contentLayout->sizeHint().height();

    for (int i = 0; i < m_toggleAnimation->animationCount() - 1; ++i) {
        QPropertyAnimation* anim =
            static_cast<QPropertyAnimation*>(m_toggleAnimation->animationAt(i));
        anim->setStartValue(collapsedHeight);
        anim->setEndValue(collapsedHeight + contentHeight);
    }

    QPropertyAnimation* contentAnimation = static_cast<QPropertyAnimation*>(
        m_toggleAnimation->animationAt(m_toggleAnimation->animationCount() - 1));
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);
}

void CollapsibleWidget::toggle(bool collapsed) {
    m_toggleButton->setArrowType(collapsed ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    m_toggleAnimation->setDirection(collapsed ? QAbstractAnimation::Forward
                                              : QAbstractAnimation::Backward);
    m_toggleAnimation->start();
}
