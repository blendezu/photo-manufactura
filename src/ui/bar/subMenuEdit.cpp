#include "subMenuEdit.h"

#include <QAction>
#include <QMessageBox>

// Constructor
SubMenuEdit::SubMenuEdit(QWidget* parent) : QMenu(parent) {
    this->setTitle("Edit");

    undoAction = new QAction("Undo", this);
    undoAction->setShortcut(QKeySequence::Undo);
    // Toggle for undo action
    undoAction->toggle();
    redoAction = new QAction("Redo", this);
    redoAction->setShortcut(QKeySequence::Redo);

    // Connect actions to slots
    connect(undoAction, &QAction::triggered, this, &SubMenuEdit::onUndoTriggered);
    connect(redoAction, &QAction::triggered, this, &SubMenuEdit::onRedoTriggered);

    // Add actions to menu
    this->addAction(undoAction);
    this->addAction(redoAction);
}

// Destructor
SubMenuEdit::~SubMenuEdit() {}

void SubMenuEdit::onUndoTriggered() {
    // TODO: Connect to ApplicationController for undo functionality
    QMessageBox::information(this, tr("Undo"), tr("Undo functionality coming soon!"));
}

void SubMenuEdit::onRedoTriggered() {
    // TODO: Connect to ApplicationController for redo functionality
    QMessageBox::information(this, tr("Redo"), tr("Redo functionality coming soon!"));
}
