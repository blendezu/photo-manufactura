#include "HistoryWidget.h"

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QListWidgetItem>
#include <QVBoxLayout>

HistoryWidget::HistoryWidget(QWidget* parent) : QGroupBox(tr("History"), parent) {
    setupUI();
}

void HistoryWidget::setupUI() {
    setStyleSheet(
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

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 10, 5, 5);

    m_listWidget = new QListWidget(this);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setStyleSheet(
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

    // Default height
    m_listWidget->setFixedHeight(150);
    layout->addWidget(m_listWidget);
}

void HistoryWidget::clear() {
    m_listWidget->clear();
}

void HistoryWidget::updateHistory(const QStringList& history) {
    m_listWidget->clear();

    if (history.isEmpty()) {
        m_listWidget->addItem(tr("Original Image"));
        return;
    }

    // Add items (history list is most recent first)
    for (const QString& action : history) {
        QListWidgetItem* item = new QListWidgetItem(action);
        // Style the most recent action differently
        if (m_listWidget->count() == 0) {
            item->setForeground(QColor("#ffffff"));
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            item->setIcon(QIcon::fromTheme("edit-undo"));
        }
        m_listWidget->addItem(item);
    }

    // Add "Original" at the bottom
    m_listWidget->addItem(tr("Original Image"));
}
