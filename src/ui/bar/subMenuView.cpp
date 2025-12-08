#include "subMenuView.h"

#include <QActionGroup>

#include "../resources/theme/themeManager.h"

SubMenuView::SubMenuView(QWidget* parent) : QMenu(parent) {
    this->setTitle("View");

    // Theme submenu
    QMenu* themeMenu = new QMenu("Theme", this);

    m_darkThemeAction = new QAction("Dark Theme", this);
    m_darkThemeAction->setIcon(
        QIcon::fromTheme("view-dark-theme"));  // Use a standard icon if available
    m_darkThemeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setChecked(true);

    m_lightThemeAction = new QAction("Light Theme", this);
    m_lightThemeAction->setIcon(
        QIcon::fromTheme("view-light-theme"));  // Use a standard icon if available
    m_lightThemeAction->setCheckable(true);
    m_lightThemeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    // Group actions so only one can be checked
    QActionGroup* themeGroup = new QActionGroup(this);
    themeGroup->addAction(m_darkThemeAction);
    themeGroup->addAction(m_lightThemeAction);

    themeMenu->addAction(m_darkThemeAction);
    themeMenu->addAction(m_lightThemeAction);
    themeMenu->addSeparator();

    // Toggle theme action with keyboard shortcut
    m_toggleThemeAction = new QAction("Toggle Theme", this);
    m_toggleThemeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    themeMenu->addAction(m_toggleThemeAction);

    themeMenu->addSeparator();

    // Enable/Disable theme switching
    m_enableThemeSwitchingAction = new QAction("Enable Theme Switching", this);
    m_enableThemeSwitchingAction->setCheckable(true);
    m_enableThemeSwitchingAction->setChecked(true);  // Enabled by default
    themeMenu->addAction(m_enableThemeSwitchingAction);

    // Panel visibility
    m_toggleToolPanelAction = new QAction("Show Tool Panel", this);
    m_toggleToolPanelAction->setCheckable(true);
    m_toggleToolPanelAction->setChecked(true);
    m_toggleToolPanelAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));

    m_toggleInfoPanelAction = new QAction("Show Info Panel", this);
    m_toggleInfoPanelAction->setCheckable(true);
    m_toggleInfoPanelAction->setChecked(true);
    m_toggleInfoPanelAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));

    // Add to menu
    this->addMenu(themeMenu);
    this->addSeparator();
    this->addAction(m_toggleToolPanelAction);
    this->addAction(m_toggleInfoPanelAction);

    // Connect signals
    connect(m_darkThemeAction, &QAction::triggered, this, &SubMenuView::onDarkThemeTriggered);
    connect(m_lightThemeAction, &QAction::triggered, this, &SubMenuView::onLightThemeTriggered);
    connect(m_toggleThemeAction, &QAction::triggered, this, &SubMenuView::onToggleThemeTriggered);
    connect(m_toggleToolPanelAction, &QAction::triggered, this,
            &SubMenuView::onToggleToolPanelTriggered);
    connect(m_toggleInfoPanelAction, &QAction::triggered, this,
            &SubMenuView::onToggleInfoPanelTriggered);

    // Listen to theme changes to update checkboxes
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            &SubMenuView::onThemeChanged);

    // Listen to theme switching enabled/disabled changes
    connect(&ThemeManager::instance(), &ThemeManager::themeSwitchingEnabledChanged, this,
            &SubMenuView::onThemeSwitchingEnabledChanged);

    // Connect enable/disable action
    connect(m_enableThemeSwitchingAction, &QAction::toggled, [](bool checked) {
        if (checked) {
            ThemeManager::instance().enableThemeSwitching();
        } else {
            ThemeManager::instance().disableThemeSwitching();
        }
    });

    // Initialize theme state from current theme
    onThemeChanged(ThemeManager::instance().currentTheme());
    onThemeSwitchingEnabledChanged(ThemeManager::instance().isThemeSwitchingEnabled());
}

SubMenuView::~SubMenuView() {}

void SubMenuView::onDarkThemeTriggered() {
    // TODO: MOVE TO CONTROLLER: Theme management should be handled by ApplicationController
    // The controller should handle:
    //   1. Persisting theme preference to settings
    //   2. Managing theme state via setState("theme", value)
    //   3. Coordinating theme application across all UI components
    // The UI should emit a themeChangeRequested(Theme::Dark) signal

    ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);
}

void SubMenuView::onLightThemeTriggered() {
    // TODO: MOVE TO CONTROLLER: Theme management should be handled by ApplicationController
    // See onDarkThemeTriggered() comments for details

    ThemeManager::instance().applyTheme(ThemeManager::Theme::Light);
}

void SubMenuView::onToggleThemeTriggered() {
    ThemeManager::instance().toggleTheme();
}

void SubMenuView::onThemeChanged(ThemeManager::Theme theme) {
    // Update menu checkboxes to reflect current theme
    m_darkThemeAction->setChecked(theme == ThemeManager::Theme::Dark);
    m_lightThemeAction->setChecked(theme == ThemeManager::Theme::Light);
}

void SubMenuView::onThemeSwitchingEnabledChanged(bool enabled) {
    // Enable or disable theme-related actions
    m_darkThemeAction->setEnabled(enabled);
    m_lightThemeAction->setEnabled(enabled);
    m_toggleThemeAction->setEnabled(enabled);
    m_enableThemeSwitchingAction->setChecked(enabled);
}

void SubMenuView::onToggleToolPanelTriggered() {
    // TODO: MOVE TO CONTROLLER: Panel visibility state should be managed by ApplicationController
    // The controller should handle:
    //   1. Managing panel visibility state via setState("toolPanelVisible", value)
    //   2. Persisting panel preferences to settings
    //   3. Coordinating with MainWindow to show/hide dock widgets
    // The UI should emit a toggleToolPanelRequested() signal

    // Will be connected to main window's dock widget visibility
    // For now, just a placeholder
}

void SubMenuView::onToggleInfoPanelTriggered() {
    // TODO: MOVE TO CONTROLLER: Panel visibility state should be managed by ApplicationController
    // See onToggleToolPanelTriggered() comments for details

    // Will be connected to main window's dock widget visibility
    // For now, just a placeholder
}
