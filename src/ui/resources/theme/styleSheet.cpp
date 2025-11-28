#include "styleSheet.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>

QString StyleSheet::loadQssFile(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to open stylesheet file:" << resourcePath;
        return QString();
    }

    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    file.close();

    return styleSheet;
}

QString StyleSheet::loadTheme(Theme theme) {
    switch (theme) {
        case Theme::Dark:
            return loadQssFile(":/styles/styles/dark_theme.qss");
        case Theme::Light:
            return loadQssFile(":/styles/styles/light_theme.qss");
        default:
            return loadQssFile(":/styles/styles/dark_theme.qss");
    }
}

QString StyleSheet::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to open stylesheet file:" << filePath;
        return QString();
    }

    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    file.close();

    return styleSheet;
}

QString StyleSheet::getDarkTheme() {
    return loadTheme(Theme::Dark);
}

QString StyleSheet::getLightTheme() {
    return loadTheme(Theme::Light);
}

QString StyleSheet::getLightroomStyle() {
    return getDarkTheme();
}
