#pragma once

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
   private slots:
    void onSelectImage();
    void onProcessImage();

   private:
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QHBoxLayout* inputLayout;
    QLabel* imageLabel;
    QLineEdit* imagePathEdit;
    QPushButton* selectImageButton;
    QPushButton* processImageButton;
};