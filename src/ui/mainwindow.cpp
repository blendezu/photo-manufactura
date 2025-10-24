#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    centralWidget = new QWidget(this);
    mainLayout = new QVBoxLayout(centralWidget);
    inputLayout = new QHBoxLayout();

    imageLabel = new QLabel("Image Path:", this);
    imagePathEdit = new QLineEdit(this);
    selectImageButton = new QPushButton("Select Image", this);
    processImageButton = new QPushButton("Process Image", this);

    inputLayout->addWidget(imageLabel);
    inputLayout->addWidget(imagePathEdit);
    inputLayout->addWidget(selectImageButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(processImageButton);

    setCentralWidget(centralWidget);

    connect(selectImageButton, &QPushButton::clicked, this, &MainWindow::onSelectImage);
    connect(processImageButton, &QPushButton::clicked, this, &MainWindow::onProcessImage);
}

MainWindow::~MainWindow() {}

void MainWindow::onSelectImage() {
    QString fileName =
        QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        imagePathEdit->setText(fileName);
    }
}

void MainWindow::onProcessImage() {
    QString imagePath = imagePathEdit->text();
    if (imagePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select an image first.");
        return;
    }
    // Placeholder for image processing logic
    QMessageBox::information(this, "Info", "Processing image: " + imagePath);
}
