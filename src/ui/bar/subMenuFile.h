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

    QString m_currentFilePath;
};