#pragma once

#include <QAction>
#include <QMenu>

#include "../resources/theme/themeManager.h"

class SubMenuView : public QMenu {
    Q_OBJECT
   public:
    explicit SubMenuView(QWidget* parent = nullptr);
    ~SubMenuView();

   private slots:
    void onDarkThemeTriggered();
    void onLightThemeTriggered();
    void onToggleThemeTriggered();
    void onToggleToolPanelTriggered();
    void onToggleInfoPanelTriggered();
    void onThemeChanged(ThemeManager::Theme theme);
    void onThemeSwitchingEnabledChanged(bool enabled);

   private:
    QAction* m_darkThemeAction;
    QAction* m_lightThemeAction;
    QAction* m_toggleThemeAction;
    QAction* m_enableThemeSwitchingAction;
    QAction* m_toggleToolPanelAction;
    QAction* m_toggleInfoPanelAction;
};
