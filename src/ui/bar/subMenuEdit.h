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
    // Slots for actions
   private slots:
    void onUndoTriggered();
    void onRedoTriggered();
    // Actions :
   private:
    QAction* undoAction;
    QAction* redoAction;
};