#include "subMenuFile.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>

SubMenuFile::SubMenuFile(QWidget* parent) : QMenu(parent) {
    this->setTitle("File");

    QAction* newAction = new QAction("New", this);
    QAction* openAction = new QAction("Open", this);
    QAction* saveAction = new QAction("Save", this);
    QAction* exitAction = new QAction("Exit", this);

    this->addAction(newAction);
    this->addAction(openAction);
    this->addAction(saveAction);
    this->addSeparator();
    this->addAction(exitAction);

    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
}

SubMenuFile::~SubMenuFile() {}

void SubMenuFile::onNewTriggered() {}
void SubMenuFile::onOpenTriggered() {
    // Open file dialog
    QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                 tr("Images (*.png *.xpm *.jpg);;All Files (*)"));
    // Handle the selected file
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open File"), "", tr("Images (*.png *.xpm *.jpg);;All Files (*)"));
    if (!fileName.isEmpty()) {
        // Load the file or open canvasWidget with the file
    if (!fileName.isEmpty()) {
        // Load the file or open canvasWidget with the file
        // TODO: Set canvasWidget reference and implement loadImage
        // canvasWidget->loadImage(fileName);
    }
    // Handle Save action
}
void SubMenuFile::onExportTriggered() {
    // Handle Export action
}