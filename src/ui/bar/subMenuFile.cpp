#include "subMenuFile.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

SubMenuFile::SubMenuFile(QWidget* parent) : QMenu(parent) {
    this->setTitle("File");

    // Create actions with shortcuts
    newAction = new QAction("New", this);
    newAction->setShortcut(QKeySequence::New);

    openAction = new QAction("Open...", this);
    openAction->setShortcut(QKeySequence::Open);

    saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence::Save);

    exitAction = new QAction("Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);

    // Connect actions to slots
    connect(newAction, &QAction::triggered, this, &SubMenuFile::onNewTriggered);
    connect(openAction, &QAction::triggered, this, &SubMenuFile::onOpenTriggered);
    connect(saveAction, &QAction::triggered, this, &SubMenuFile::onSaveTriggered);
    connect(exitAction, &QAction::triggered, this, &SubMenuFile::onExitTriggered);

    // Add actions to menu
    this->addAction(newAction);
    this->addAction(openAction);
    this->addAction(saveAction);
    this->addSeparator();
    this->addAction(exitAction);
}

SubMenuFile::~SubMenuFile() {}

void SubMenuFile::onNewTriggered() {
    // TODO: Connect to ApplicationController to create new document
    QMessageBox::information(this, tr("New"), tr("New document functionality coming soon!"));
}

void SubMenuFile::onOpenTriggered() {
    // Open file dialog
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Image"), "",
        tr("Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.raw *.cr2 *.nef);;All Files (*)"));

    if (!fileName.isEmpty()) {
        // TODO: Connect to ApplicationController to load image
        QMessageBox::information(this, tr("Open"), tr("Opening: %1").arg(fileName));
    }
}

void SubMenuFile::onSaveTriggered() {
    // Open file dialog for saving
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Image"), "",
        tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tif *.tiff);;All Files (*)"));

    if (!fileName.isEmpty()) {
        // TODO: Connect to ApplicationController to save image
        QMessageBox::information(this, tr("Save"), tr("Saving to: %1").arg(fileName));
    }
}

void SubMenuFile::onExitTriggered() {
    QApplication::quit();
}