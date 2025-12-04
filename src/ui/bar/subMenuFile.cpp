#include "subMenuFile.h"

#include <QAction>
#include <QApplication>

SubMenuFile::SubMenuFile(QWidget* parent) : QMenu(parent) {
    this->setTitle("File");

    // Create actions with shortcuts
    newAction = new QAction("New", this);
    newAction->setShortcut(QKeySequence::New);

    openAction = new QAction("Open...", this);
    openAction->setShortcut(QKeySequence::Open);

    saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence::Save);

    saveAsAction = new QAction("Save As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);

    exitAction = new QAction("Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);

    // Connect actions to slots
    connect(newAction, &QAction::triggered, this, &SubMenuFile::onNewTriggered);
    connect(openAction, &QAction::triggered, this, &SubMenuFile::onOpenTriggered);
    connect(saveAction, &QAction::triggered, this, &SubMenuFile::onSaveTriggered);
    connect(saveAsAction, &QAction::triggered, this, &SubMenuFile::onSaveAsTriggered);
    connect(exitAction, &QAction::triggered, this, &SubMenuFile::onExitTriggered);

    // Add actions to menu
    this->addAction(newAction);
    this->addAction(openAction);
    this->addAction(saveAction);
    this->addAction(saveAsAction);
    this->addSeparator();
    this->addAction(exitAction);
}

SubMenuFile::~SubMenuFile() {}

void SubMenuFile::onNewTriggered() {
    emit newDocumentRequested();
}

void SubMenuFile::onOpenTriggered() {
    emit openFileRequested();
}

void SubMenuFile::onSaveTriggered() {
    emit saveFileRequested();
}

void SubMenuFile::onSaveAsTriggered() {
    emit saveAsFileRequested();
}

void SubMenuFile::onExitTriggered() {
    emit exitRequested();
}