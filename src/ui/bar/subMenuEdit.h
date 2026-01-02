#pragma once

#include <QAction>
#include <QMenu>

class SubMenuEdit : public QMenu {
    Q_OBJECT
   public:
    // Constructor
    explicit SubMenuEdit(QWidget* parent = nullptr);
    // Destructor
    ~SubMenuEdit();

   Q_SIGNALS:
    // Edit signals
    void undoRequested();
    void redoRequested();

    // Geometry signals
    void rotateLeftRequested();   // -90 degrees
    void rotateRightRequested();  // +90 degrees
    void flipHorizontalRequested();
    void flipVerticalRequested();

   private:
    // Undo/Redo actions
    QAction* undoAction;
    QAction* redoAction;

    // Geometry actions
    QAction* rotateLeftAction;
    QAction* rotateRightAction;
    QAction* flipHorizontalAction;
    QAction* flipVerticalAction;
};