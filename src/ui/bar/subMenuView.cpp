#include "subMenuView.h"

#include <QActionGroup>

#include "../resources/theme/themeManager.h"

SubMenuView::SubMenuView(QWidget* parent) : QMenu(parent) {
    this->setTitle("View");

    // Zoom submenu
    QMenu* zoomMenu = new QMenu("Zoom", this);

    m_zoomInAction = new QAction("Zoom In", this);
    m_zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    connect(m_zoomInAction, &QAction::triggered, this, &SubMenuView::zoomInRequested);

    m_zoomOutAction = new QAction("Zoom Out", this);
    m_zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(m_zoomOutAction, &QAction::triggered, this, &SubMenuView::zoomOutRequested);

    m_resetZoomAction = new QAction("Actual Size (100%)", this);
    m_resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(m_resetZoomAction, &QAction::triggered, this, &SubMenuView::resetZoomRequested);

    m_fitToWindowAction = new QAction("Fit to Window", this);
    m_fitToWindowAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_9));
    connect(m_fitToWindowAction, &QAction::triggered, this, &SubMenuView::fitToWindowRequested);

    zoomMenu->addAction(m_zoomInAction);
    zoomMenu->addAction(m_zoomOutAction);
    zoomMenu->addSeparator();
    zoomMenu->addAction(m_resetZoomAction);
    zoomMenu->addAction(m_fitToWindowAction);

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
    m_toggleToolPanelAction->setShortcut(QKeySequence(Qt::Key_Tab));

    m_toggleHistogramAction = new QAction("Show Histogram", this);
    m_toggleHistogramAction->setCheckable(true);
    m_toggleHistogramAction->setChecked(true);
    m_toggleHistogramAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    connect(m_toggleHistogramAction, &QAction::triggered, this,
            &SubMenuView::toggleHistogramRequested);

    m_toggleInfoPanelAction = new QAction("Show Adjustment Panel", this);
    m_toggleInfoPanelAction->setCheckable(true);
    m_toggleInfoPanelAction->setChecked(true);
    m_toggleInfoPanelAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));

    // Add to menu
    this->addMenu(zoomMenu);
    this->addSeparator();
    this->addMenu(themeMenu);
    this->addSeparator();
    this->addAction(m_toggleToolPanelAction);
    this->addAction(m_toggleHistogramAction);
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
    // Emit signal for controller to handle, also apply directly for immediate feedback
    ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);
    emit darkThemeRequested();
}

void SubMenuView::onLightThemeTriggered() {
    // Emit signal for controller to handle, also apply directly for immediate feedback
    ThemeManager::instance().applyTheme(ThemeManager::Theme::Light);
    emit lightThemeRequested();
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
    // Emit signal for controller to coordinate with MainWindow dock widgets
    emit toggleToolPanelRequested();
}

void SubMenuView::onToggleInfoPanelTriggered() {
    // Emit signal for controller to coordinate with MainWindow dock widgets
    emit toggleAdjustmentPanelRequested();
}
