#include "subMenuFile.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

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

    // TODO: MOVE TO CONTROLLER: This entire slot should delegate to
    // ApplicationController::createNewDocument() The UI should only emit a signal, and the
    // controller should handle document creation logic
}

void SubMenuFile::onOpenTriggered() {
    // TODO: MOVE TO CONTROLLER: The file dialog should be opened by
    // ApplicationController::openFile() The controller should handle:
    //   1. Opening the file dialog (QFileDialog)
    //   2. Validating the file path
    //   3. Managing m_currentFilePath state
    //   4. Coordinating with DocumentManager to load the file
    // The UI should only emit a signal like openRequested() and wait for controller response

    // Load last used directory
    QSettings settings("PhotoManufactura", "UI");
    QString lastDir = settings.value("lastImageDirectory", "").toString();

    // Open file dialog
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Image"), lastDir,
        tr("Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.raw *.cr2 *.nef);;All Files (*)"));

    if (!fileName.isEmpty()) {
        // Save the directory for next time
        QFileInfo fileInfo(fileName);
        settings.setValue("lastImageDirectory", fileInfo.absolutePath());

        m_currentFilePath =
            fileName;  // TODO: MOVE TO CONTROLLER: State should be managed by ApplicationController
        emit imageLoaded(fileName);
    }
}

void SubMenuFile::onSaveTriggered() {
    // TODO: MOVE TO CONTROLLER: The entire save logic should be handled by
    // ApplicationController::saveFile() The controller should handle:
    //   1. Checking if current file exists (state management)
    //   2. Opening save dialog if needed (QFileDialog)
    //   3. Coordinating with DocumentManager to save the file
    //   4. Managing file path state
    // The UI should only emit a signal like saveRequested() and the controller decides save vs
    // saveAs

    // If no current file, prompt for save location
    if (m_currentFilePath.isEmpty()) {  // TODO: MOVE TO CONTROLLER: Use
                                        // ApplicationController::getState("currentFile")
        QString fileName = QFileDialog::getSaveFileName(
            this, tr("Save Image"), "",
            tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tif *.tiff);;All Files (*)"));

        if (!fileName.isEmpty()) {
            m_currentFilePath = fileName;  // TODO: MOVE TO CONTROLLER: State management
            emit imageSaveRequested(fileName);
        }
    } else {
        emit imageSaveRequested(m_currentFilePath);
    }
}

void SubMenuFile::onExitTriggered() {
    // TODO: MOVE TO CONTROLLER: Exit logic should be handled by
    // ApplicationController::exitApplication() The controller should handle:
    //   1. Checking for unsaved changes
    //   2. Prompting user to save if needed
    //   3. Cleanup and shutdown coordination
    // The UI should only emit an exitRequested() signal

    QApplication::quit();  // TODO: MOVE TO CONTROLLER: Replace with emit exitRequested()
}