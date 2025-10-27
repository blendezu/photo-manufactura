#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class InfoPanel : public QWidget {
    Q_OBJECT
   public:
    explicit InfoPanel(QWidget* parent = nullptr);
    ~InfoPanel();

   private:
    // Private members and methods can be added here
    QLabel* imageLabel;
    QVBoxLayout* layout;
};