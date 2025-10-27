#pragma once

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

class ToolBar : public QToolBar {
    Q_OBJECT
   public:
    explicit ToolBar(QWidget* parent = nullptr);
    ~ToolBar() override;

   private:
    // top-level actions (kept for non-dropdown items or reuse)
    QAction* fileAction;
    QAction* editAction;
    QAction* viewAction;
    QAction* imageAction;

    // helper to build menu buttons
    QToolButton* createMenuButton(const QString& text, QMenu* menu);

   private slots:
    void onFileActionTriggered();
    void onEditActionTriggered();
    void onViewActionTriggered();
    void onImageActionTriggered();
};
