#include "subMenuEdit.h"

#include <QAction>
#include <QKeySequence>

// Constructor
SubMenuEdit::SubMenuEdit(QWidget* parent) : QMenu(parent) {
    this->setTitle("Edit");

    // === Undo/Redo Actions ===
    undoAction = new QAction("Undo", this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setIcon(QIcon::fromTheme("edit-undo"));
    connect(undoAction, &QAction::triggered, this, &SubMenuEdit::undoRequested);

    redoAction = new QAction("Redo", this);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setIcon(QIcon::fromTheme("edit-redo"));
    connect(redoAction, &QAction::triggered, this, &SubMenuEdit::redoRequested);

    this->addAction(undoAction);
    this->addAction(redoAction);
}

// Destructor
SubMenuEdit::~SubMenuEdit() {}
