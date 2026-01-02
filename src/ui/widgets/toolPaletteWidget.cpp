#include "toolPaletteWidget.h"

#include <QPushButton>

ToolPaletteWidget::ToolPaletteWidget(const QString& title, QWidget* parent) : QWidget(parent) {
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
}

void ToolPaletteWidget::addToolButton(const QString& toolName, const QString& iconPath) {
    QPushButton* button = new QPushButton(this);
    button->setObjectName(toolName);
    button->setToolTip(toolName);
    button->setCheckable(false);  // Instant action buttons

    if (!iconPath.isEmpty()) {
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(20, 20));
    } else {
        button->setText(toolName);
    }

    button->setFixedSize(36, 36);
    button->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            border: 1px solid #555;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
            border-color: #666;
        }
        QPushButton:pressed {
            background-color: #2a2a2a;
        }
    )");

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

    // Emit the tool activation signal with the tool name
    Q_EMIT toolActivated(clickedButton->objectName());
}
