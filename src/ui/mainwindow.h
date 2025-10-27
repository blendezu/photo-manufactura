#pragma once

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

   private:
    void setupUi();
    void setupMenus();
};