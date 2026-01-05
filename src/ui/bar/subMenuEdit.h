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

    // State update slots
    void setUndoEnabled(bool enabled);
    void setRedoEnabled(bool enabled);

   private:
    // Undo/Redo actions
    QAction* undoAction;
    QAction* redoAction;
};