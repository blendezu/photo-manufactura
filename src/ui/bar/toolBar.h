#pragma once

#include <QToolBar>

class ToolBar : public QToolBar {
    Q_OBJECT
   public:
    explicit ToolBar(QWidget* parent = nullptr);
    ~ToolBar();
    // Slots for handling toolbar actions
   private slots:
    void onFileActionTriggered();
    void onEditActionTriggered();
    void onViewActionTriggered();
    void onImageActionTriggered();
    void onToolsActionTriggered();
    void onHelpActionTriggered();

   private:
    QToolBar* toolBar;
    QAction* fileAction;
    QAction* editAction;
    QAction* viewAction;
    QAction* imageAction;
    QAction* toolsAction;
    QAction* helpAction;
};