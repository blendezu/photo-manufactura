#pragma once

#include <QAction>
#include <QMenu>

class SubMenuView : public QMenu {
    Q_OBJECT
   public:
    explicit SubMenuView(QWidget* parent = nullptr);
    ~SubMenuView();

   private slots:
    void onDarkThemeTriggered();
    void onLightThemeTriggered();
    void onToggleToolPanelTriggered();
    void onToggleInfoPanelTriggered();

   private:
    QAction* m_darkThemeAction;
    QAction* m_lightThemeAction;
    QAction* m_toggleToolPanelAction;
    QAction* m_toggleInfoPanelAction;
};
