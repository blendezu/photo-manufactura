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
    this->addSeparator();

    // === Transform/Geometry Actions ===
    rotateLeftAction = new QAction("Rotate Left", this);
    rotateLeftAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));  // Cmd+[
    rotateLeftAction->setIcon(QIcon::fromTheme("object-rotate-left"));
    connect(rotateLeftAction, &QAction::triggered, this, &SubMenuEdit::rotateLeftRequested);

    rotateRightAction = new QAction("Rotate Right", this);
    rotateRightAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));  // Cmd+]
    rotateRightAction->setIcon(QIcon::fromTheme("object-rotate-right"));
    connect(rotateRightAction, &QAction::triggered, this, &SubMenuEdit::rotateRightRequested);

    flipHorizontalAction = new QAction("Flip Horizontal", this);
    flipHorizontalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    flipHorizontalAction->setIcon(QIcon::fromTheme("object-flip-horizontal"));
    connect(flipHorizontalAction, &QAction::triggered, this, &SubMenuEdit::flipHorizontalRequested);

    flipVerticalAction = new QAction("Flip Vertical", this);
    flipVerticalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
    flipVerticalAction->setIcon(QIcon::fromTheme("object-flip-vertical"));
    connect(flipVerticalAction, &QAction::triggered, this, &SubMenuEdit::flipVerticalRequested);

    this->addAction(rotateLeftAction);
    this->addAction(rotateRightAction);
    this->addSeparator();
    this->addAction(flipHorizontalAction);
    this->addAction(flipVerticalAction);
}

// Destructor
SubMenuEdit::~SubMenuEdit() {}
