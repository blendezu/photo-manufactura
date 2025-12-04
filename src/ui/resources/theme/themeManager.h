#pragma once

#include <QObject>
#include <QString>

/**
 * @brief Theme Manager for the application
 *
 * Manages theme switching, persistence, and application-wide theme changes.
 * Emits signals when theme changes to notify interested components.
 */
class ThemeManager : public QObject {
    Q_OBJECT

   public:
    enum class Theme {
        Dark,
        Light,
        Auto  // Could follow system theme
    };

    static ThemeManager& instance();

    /**
     * @brief Apply theme to the entire application
     * @param theme The theme to apply
     */
    void applyTheme(Theme theme);

    /**
     * @brief Get current active theme
     * @return Current theme
     */
    Theme currentTheme() const {
        return m_currentTheme;
    }

    /**
     * @brief Load theme from settings
     */
    void loadThemeFromSettings();

    /**
     * @brief Save current theme to settings
     */
    void saveThemeToSettings();

    /**
     * @brief Toggle between dark and light theme
     */
    void toggleTheme();

    /**
     * @brief Check if dark theme is active
     * @return true if dark theme is active
     */
    bool isDarkTheme() const {
        return m_currentTheme == Theme::Dark;
    }

    /**
     * @brief Enable theme switching
     */
    void enableThemeSwitching();

    /**
     * @brief Disable theme switching (lock current theme)
     */
    void disableThemeSwitching();

    /**
     * @brief Check if theme switching is enabled
     * @return true if theme switching is enabled
     */
    bool isThemeSwitchingEnabled() const {
        return m_themeSwitchingEnabled;
    }

   signals:
    void themeChanged(Theme theme);
    void themeSwitchingEnabledChanged(bool enabled);

   private:
    ThemeManager();
    ~ThemeManager() = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    Theme m_currentTheme;
    bool m_themeSwitchingEnabled;
};
