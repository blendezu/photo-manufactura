#include "subMenuView.h"

#include <QActionGroup>

#include "../widgets/themeManager.h"

SubMenuView::SubMenuView(QWidget* parent) : QMenu(parent) {
    this->setTitle("View");

    // Theme submenu
    QMenu* themeMenu = new QMenu("Theme", this);

    m_darkThemeAction = new QAction("Dark Theme", this);
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setChecked(true);

    m_lightThemeAction = new QAction("Light Theme", this);
    m_lightThemeAction->setCheckable(true);

    // Group actions so only one can be checked
    QActionGroup* themeGroup = new QActionGroup(this);
    themeGroup->addAction(m_darkThemeAction);
    themeGroup->addAction(m_lightThemeAction);

    themeMenu->addAction(m_darkThemeAction);
    themeMenu->addAction(m_lightThemeAction);

    // Panel visibility
    m_toggleToolPanelAction = new QAction("Show Tool Panel", this);
    m_toggleToolPanelAction->setCheckable(true);
    m_toggleToolPanelAction->setChecked(true);
    m_toggleToolPanelAction->setShortcut(Qt::Key_T);

    m_toggleInfoPanelAction = new QAction("Show Info Panel", this);
    m_toggleInfoPanelAction->setCheckable(true);
    m_toggleInfoPanelAction->setChecked(true);
    m_toggleInfoPanelAction->setShortcut(Qt::Key_I);

    // Add to menu
    this->addMenu(themeMenu);
    this->addSeparator();
    this->addAction(m_toggleToolPanelAction);
    this->addAction(m_toggleInfoPanelAction);

    // Connect signals
    connect(m_darkThemeAction, &QAction::triggered, this, &SubMenuView::onDarkThemeTriggered);
    connect(m_lightThemeAction, &QAction::triggered, this, &SubMenuView::onLightThemeTriggered);
    connect(m_toggleToolPanelAction, &QAction::triggered, this,
            &SubMenuView::onToggleToolPanelTriggered);
    connect(m_toggleInfoPanelAction, &QAction::triggered, this,
            &SubMenuView::onToggleInfoPanelTriggered);
}

SubMenuView::~SubMenuView() {}

void SubMenuView::onDarkThemeTriggered() {
    ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);
}

void SubMenuView::onLightThemeTriggered() {
    ThemeManager::instance().applyTheme(ThemeManager::Theme::Light);
}

void SubMenuView::onToggleToolPanelTriggered() {
    // Will be connected to main window's dock widget visibility
    // For now, just a placeholder
}

void SubMenuView::onToggleInfoPanelTriggered() {
    // Will be connected to main window's dock widget visibility
    // For now, just a placeholder
}
