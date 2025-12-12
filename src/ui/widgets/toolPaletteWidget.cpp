#include "toolPaletteWidget.h"

#include <QPushButton>

ToolPaletteWidget::ToolPaletteWidget(const QString& title, QWidget* parent) : QWidget(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(8);
}

void ToolPaletteWidget::addToolButton(const QString& toolName, const QString& iconPath) {
    QPushButton* button = new QPushButton(toolName, this);
    button->setCheckable(true);
    button->setIcon(QIcon(iconPath));
    button->setIconSize(QSize(24, 24));
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, &ToolPaletteWidget::onToolButtonClicked);
}

bool ToolPaletteWidget::isToolButtonChecked(const QString& toolName) {
    for (int i = 0; i < m_layout->count(); ++i) {
        QWidget* widget = m_layout->itemAt(i)->widget();
        if (QPushButton* button = qobject_cast<QPushButton*>(widget)) {
            if (button->text() == toolName) {
                return button->isChecked();
            }
        }
    }
    return false;
}

void ToolPaletteWidget::setToolButtonChecked(const QString& toolName, bool checked) {
    for (int i = 0; i < m_layout->count(); ++i) {
        QWidget* widget = m_layout->itemAt(i)->widget();
        if (QPushButton* button = qobject_cast<QPushButton*>(widget)) {
            if (button->text() == toolName) {
                button->setChecked(checked);
                return;
            }
        }
    }
}
void ToolPaletteWidget::onToolButtonClicked() {
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton)
        return;

    // Uncheck all other buttons
    for (int i = 0; i < m_layout->count(); ++i) {
        QWidget* widget = m_layout->itemAt(i)->widget();
        if (QPushButton* button = qobject_cast<QPushButton*>(widget)) {
            if (button != clickedButton) {
                button->setChecked(false);
            }
        }
    }
}

