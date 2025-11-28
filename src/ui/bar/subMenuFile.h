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
    void imageLoaded(const QString& filePath);
    void imageSaveRequested(const QString& filePath);

   private slots:
    void onNewTriggered();
    void onOpenTriggered();
    void onSaveTriggered();
    void onExitTriggered();

   private:
    QAction* newAction;
    QAction* openAction;
    QAction* saveAction;
    QAction* exitAction;

    // TODO: MOVE TO CONTROLLER: m_currentFilePath should be managed by ApplicationController state
    // Use ApplicationController::getState("currentFile") instead
    QString m_currentFilePath;
};