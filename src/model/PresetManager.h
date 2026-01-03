#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

#include "AdjustmentSettings.h"

/**
 * @brief Manages image adjustment presets
 *
 * Handles saving, loading, and managing named presets for image adjustments.
 * Presets are stored as JSON files in the application's data directory.
 */
class PresetManager : public QObject {
    Q_OBJECT

   public:
    explicit PresetManager(QObject* parent = nullptr);
    ~PresetManager() = default;

    // Built-in presets
    static QStringList builtInPresets();
    static QJsonObject getBuiltInPreset(const QString& name);

    // User presets
    QStringList userPresets() const;
    bool savePreset(const QString& name, const AdjustmentSettings* settings);
    bool loadPreset(const QString& name, AdjustmentSettings* settings);
    bool deletePreset(const QString& name);
    bool presetExists(const QString& name) const;

    // All presets (built-in + user)
    QStringList allPresets() const;

   Q_SIGNALS:
    void presetSaved(const QString& name);
    void presetDeleted(const QString& name);
    void presetLoaded(const QString& name);

   private:
    QString presetsDirectory() const;
    QString presetFilePath(const QString& name) const;
    QJsonObject settingsToJson(const AdjustmentSettings* settings) const;
    void jsonToSettings(const QJsonObject& json, AdjustmentSettings* settings) const;
};
