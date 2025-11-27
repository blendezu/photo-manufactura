#pragma once

#include <QString>

/**
 * @brief StyleSheet manager for loading and applying Qt Style Sheets
 *
 * Loads .qss files from resources or filesystem and provides
 * easy access to different themes.
 */
class StyleSheet {
   public:
    enum class Theme { Dark, Light };

    /**
     * @brief Load stylesheet from Qt resources
     * @param theme The theme to load (Dark or Light)
     * @return QString containing the stylesheet content
     */
    static QString loadTheme(Theme theme);

    /**
     * @brief Load stylesheet from file path
     * @param filePath Absolute path to .qss file
     * @return QString containing the stylesheet content
     */
    static QString loadFromFile(const QString& filePath);

    /**
     * @brief Get dark theme stylesheet
     * @return QString containing dark theme styles
     */
    static QString getDarkTheme();

    /**
     * @brief Get light theme stylesheet
     * @return QString containing light theme styles
     */
    static QString getLightTheme();

    /**
     * @brief Get Lightroom-style theme (alias for dark theme)
     * @return QString containing Lightroom-style styles
     */
    static QString getLightroomStyle();

   private:
    static QString loadQssFile(const QString& resourcePath);
};
