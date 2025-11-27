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

   signals:
    void themeChanged(Theme theme);

   private:
    ThemeManager();
    ~ThemeManager() = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    Theme m_currentTheme;
};
