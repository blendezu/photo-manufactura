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

    // TODO: Move the slots to ApplicationController
    connect(undoAction, &QAction::triggered, this, &SubMenuEdit::onUndoTriggered);
    connect(redoAction, &QAction::triggered, this, &SubMenuEdit::onRedoTriggered);

    // Add actions to menu
    this->addAction(undoAction);
    this->addAction(redoAction);
}

// Destructor
SubMenuEdit::~SubMenuEdit() {}

void SubMenuEdit::onUndoTriggered() {
    // TODO: MOVE TO CONTROLLER: Undo logic should be handled by ApplicationController::undo()
    // The controller should handle:
    //   1. Managing the undo/redo stack
    //   2. Coordinating with DocumentManager to revert changes
    //   3. Updating application state
    // The UI should only emit an undoRequested() signal

    // TODO: Connect to ApplicationController for undo functionality
    QMessageBox::information(this, tr("Undo"), tr("Undo functionality coming soon!"));
}

void SubMenuEdit::onRedoTriggered() {
    // TODO: MOVE TO CONTROLLER: Redo logic should be handled by ApplicationController::redo()
    // The controller should handle:
    //   1. Managing the undo/redo stack
    //   2. Coordinating with DocumentManager to reapply changes
    //   3. Updating application state
    // The UI should only emit a redoRequested() signal

    // TODO: Connect to ApplicationController for redo functionality
    QMessageBox::information(this, tr("Redo"), tr("Redo functionality coming soon!"));
}
