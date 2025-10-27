#pragma once

#include <QWidget>
class ToolPanel : public QWidget {
    Q_OBJECT
   public:
    explicit ToolPanel(QWidget* parent = nullptr);
    ~ToolPanel();

   signals:
   private:
    // Private members (if needed)
};