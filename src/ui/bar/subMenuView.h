#pragma once

#include <QAction>
#include <QMenu>

#include "../resources/theme/themeManager.h"

class SubMenuView : public QMenu {
    Q_OBJECT
   public:
    explicit SubMenuView(QWidget* parent = nullptr);
    ~SubMenuView();

   signals:
    // Signals for controller to handle
    void zoomInRequested();
    void zoomOutRequested();
    void resetZoomRequested();
    void fitToWindowRequested();
    void toggleHistogramRequested();
    void toggleToolPanelRequested();
    void toggleAdjustmentPanelRequested();
    void darkThemeRequested();
    void lightThemeRequested();

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

    // Zoom actions
    QAction* m_zoomInAction;
    QAction* m_zoomOutAction;
    QAction* m_resetZoomAction;
    QAction* m_fitToWindowAction;
    QAction* m_toggleHistogramAction;
};
