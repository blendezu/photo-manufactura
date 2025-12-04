#include "themeManager.h"

#include <QApplication>
#include <QSettings>

#include "styleSheet.h"

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() : m_currentTheme(Theme::Dark), m_themeSwitchingEnabled(true) {
    loadThemeFromSettings();
}

void ThemeManager::applyTheme(Theme theme) {
    if (!m_themeSwitchingEnabled) {
        return;  // Theme switching is disabled
    }

    m_currentTheme = theme;

    QString stylesheet;
    switch (theme) {
        case Theme::Dark:
            stylesheet = StyleSheet::getDarkTheme();
            break;
        case Theme::Light:
            stylesheet = StyleSheet::getLightTheme();
            break;
        case Theme::Auto:
            // Could detect system theme here
            stylesheet = StyleSheet::getDarkTheme();
            break;
    }

    if (qApp) {
        qApp->setStyleSheet(stylesheet);
    }

    emit themeChanged(theme);
    saveThemeToSettings();
}

void ThemeManager::loadThemeFromSettings() {
    QSettings settings("PhotoManufactura", "UI");
    int themeValue = settings.value("theme", static_cast<int>(Theme::Dark)).toInt();
    m_currentTheme = static_cast<Theme>(themeValue);
}

void ThemeManager::saveThemeToSettings() {
    QSettings settings("PhotoManufactura", "UI");
    settings.setValue("theme", static_cast<int>(m_currentTheme));
}

void ThemeManager::toggleTheme() {
    if (!m_themeSwitchingEnabled) {
        return;  // Theme switching is disabled
    }

    Theme newTheme = (m_currentTheme == Theme::Dark) ? Theme::Light : Theme::Dark;
    applyTheme(newTheme);
}

void ThemeManager::enableThemeSwitching() {
    if (m_themeSwitchingEnabled) {
        return;  // Already enabled
    }

    m_themeSwitchingEnabled = true;
    emit themeSwitchingEnabledChanged(true);
}

void ThemeManager::disableThemeSwitching() {
    if (!m_themeSwitchingEnabled) {
        return;  // Already disabled
    }

    m_themeSwitchingEnabled = false;
    emit themeSwitchingEnabledChanged(false);
}
