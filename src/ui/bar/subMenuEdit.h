#pragma once

#include <QAction>
#include <Qmenu>

class SubMenuEdit : public QMenu {
    Q_OBJECT
   public:
    explicit SubMenuEdit(QWidget* parent = nullptr);
    ~SubMenuEdit();

   private slots:
    void onUndoTriggered();
    void onRedoTriggered();

   private:
    QAction* undoAction;
    QAction* redoAction;
};