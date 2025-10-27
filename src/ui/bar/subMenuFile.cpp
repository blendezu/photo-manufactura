#include "subMenuFile.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>

#include "widgets/canvasWidget.h"

SubMenuFile::SubMenuFile(QWidget* parent) : QMenu(parent) {
    this->setTitle("File");

    QAction* newAction = new QAction("New", this);
    QAction* openAction = new QAction("Open", this);
    QAction* saveAction = new QAction("Save", this);
    QAction* exitAction = new QAction("Exit", this);
    QAction* someAction = new QAction();

    // TODO : Export to ApplicationController for handling actions
    this->addAction(newAction);
    this->addAction(openAction);
    this->addAction(saveAction);
    this->addSeparator();
    this->addAction(exitAction);

    // TODO: Connect actions to slots
    connect(newAction, &QAction::triggered, this, &SubMenuFile::onNewTriggered);
    connect(openAction, &QAction::triggered, this, &SubMenuFile::onOpenTriggered);
    connect(saveAction, &QAction::triggered, this, &SubMenuFile::onSaveTriggered);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
}

SubMenuFile::~SubMenuFile() {}

void SubMenuFile::onNewTriggered() {}
void SubMenuFile::onOpenTriggered() {
    // Open file dialog
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open File"), "", tr("Images (*.png *.xpm *.jpg);;All Files (*)"));

    if (!fileName.isEmpty()) {
    }
}

void SubMenuFile::onSaveTriggered() {
    // Open file dialog for saving
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save File"), "", tr("Images (*.png *.xpm *.jpg);;All Files (*)"));
    if (!fileName.isEmpty()) {
        // Save the current canvasWidget content to the selected file
        // TODO: Implement saveImage functionality
        // canvasWidget->saveImage(fileName);
    }
}
