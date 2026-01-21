#pragma once

#include <QGroupBox>
#include <QListWidget>

class HistoryWidget : public QGroupBox {
    Q_OBJECT
   public:
    explicit HistoryWidget(QWidget* parent = nullptr);
    ~HistoryWidget() = default;

   public Q_SLOTS:
    void updateHistory(const QStringList& history);
    void clear();

   private:
    void setupUI();
    QListWidget* m_listWidget;
};
