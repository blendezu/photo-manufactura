#pragma once

#include <QAction>
#include <QMenu>
#include <QToolBar>

class SubMenuFile : public QMenu {
    Q_OBJECT
   public:
    explicit SubMenuFile(QWidget* parent = nullptr);
    ~SubMenuFile();

   private slots:
    void onNewTriggered();
    void onOpenTriggered();
    void onSaveTriggered();
    void onExportTriggered();

   private:
    QAction* newAction;
    QAction* openAction;
    QAction* saveAction;
    QAction* exportAction;
};