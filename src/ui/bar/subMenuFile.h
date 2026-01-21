#pragma once

#include <QAction>
#include <QMenu>
#include <QToolBar>

class SubMenuFile : public QMenu {
    Q_OBJECT
   public:
    explicit SubMenuFile(QWidget* parent = nullptr);
    ~SubMenuFile();

   signals:
    void newDocumentRequested();
    void openFileRequested();
    void saveFileRequested();
    void saveAsFileRequested();
    void exitRequested();

   private slots:
    void onNewTriggered();
    void onOpenTriggered();
    void onSaveTriggered();
    void onSaveAsTriggered();
    void onExitTriggered();

   private:
    QAction* newAction;
    QAction* openAction;
    QAction* saveAction;
    QAction* saveAsAction;
    QAction* exitAction;
};