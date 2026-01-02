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

   private:
    // Undo/Redo actions
    QAction* undoAction;
    QAction* redoAction;
};