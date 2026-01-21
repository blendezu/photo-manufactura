#include "PresetManager.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

PresetManager::PresetManager(QObject* parent) : QObject(parent) {
    // Ensure presets directory exists
    QDir().mkpath(presetsDirectory());
}

QStringList PresetManager::builtInPresets() {
    return {"Cinematic",    "Portrait",      "Landscape",  "Warm Sunset", "Cool Blue",
            "Vintage Film", "High Contrast", "Soft Light", "Dramatic",    "Natural"};
}

QJsonObject PresetManager::getBuiltInPreset(const QString& name) {
    QJsonObject preset;

    if (name == "Cinematic") {
        preset["exposure"] = 5;
        preset["contrast"] = 15;
        preset["highlights"] = -10;
        preset["shadows"] = 15;
        preset["temperature"] = -10;
        preset["saturation"] = -5;
    } else if (name == "Portrait") {
        preset["exposure"] = 10;
        preset["contrast"] = 5;
        preset["highlights"] = -15;
        preset["shadows"] = 20;
        preset["temperature"] = 10;
        preset["saturation"] = 5;
    } else if (name == "Landscape") {
        preset["exposure"] = 0;
        preset["contrast"] = 20;
        preset["highlights"] = -20;
        preset["shadows"] = 25;
        preset["whites"] = 10;
        preset["blacks"] = -10;
        preset["saturation"] = 15;
    } else if (name == "Warm Sunset") {
        preset["exposure"] = 10;
        preset["contrast"] = 10;
        preset["temperature"] = 40;
        preset["tint"] = 10;
        preset["saturation"] = 20;
    } else if (name == "Cool Blue") {
        preset["exposure"] = 0;
        preset["contrast"] = 10;
        preset["temperature"] = -30;
        preset["tint"] = -5;
        preset["saturation"] = 10;
    } else if (name == "Vintage Film") {
        preset["exposure"] = 5;
        preset["contrast"] = -10;
        preset["highlights"] = -20;
        preset["shadows"] = 10;
        preset["whites"] = -15;
        preset["blacks"] = 15;
        preset["temperature"] = 15;
        preset["saturation"] = -15;
    } else if (name == "High Contrast") {
        preset["exposure"] = 0;
        preset["contrast"] = 40;
        preset["highlights"] = -10;
        preset["shadows"] = -10;
        preset["whites"] = 20;
        preset["blacks"] = -20;
    } else if (name == "Soft Light") {
        preset["exposure"] = 10;
        preset["contrast"] = -15;
        preset["highlights"] = -30;
        preset["shadows"] = 30;
        preset["saturation"] = -5;
    } else if (name == "Dramatic") {
        preset["exposure"] = -5;
        preset["contrast"] = 30;
        preset["highlights"] = -25;
        preset["shadows"] = 20;
        preset["whites"] = 15;
        preset["blacks"] = -25;
        preset["saturation"] = 10;
    } else if (name == "Natural") {
        // All zeros - reset to default
        preset["exposure"] = 0;
        preset["contrast"] = 0;
        preset["highlights"] = 0;
        preset["shadows"] = 0;
        preset["whites"] = 0;
        preset["blacks"] = 0;
        preset["temperature"] = 0;
        preset["tint"] = 0;
        preset["saturation"] = 0;
        preset["brightness"] = 0;
    }

    return preset;
}

QStringList PresetManager::userPresets() const {
    QDir dir(presetsDirectory());
    QStringList filters;
    filters << "*.json";
    QStringList files = dir.entryList(filters, QDir::Files);

    QStringList presets;
    for (const QString& file : files) {
        presets << file.chopped(5);  // Remove .json extension
    }
    return presets;
}

QStringList PresetManager::allPresets() const {
    QStringList all = builtInPresets();
    all << userPresets();
    return all;
}

bool PresetManager::savePreset(const QString& name, const AdjustmentSettings* settings) {
    if (!settings || name.isEmpty())
        return false;

    QJsonObject json = settingsToJson(settings);
    QJsonDocument doc(json);

    QFile file(presetFilePath(name));
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toJson());
    file.close();

    Q_EMIT presetSaved(name);
    return true;
}

bool PresetManager::loadPreset(const QString& name, AdjustmentSettings* settings) {
    if (!settings || name.isEmpty())
        return false;

    // Check built-in presets first
    if (builtInPresets().contains(name)) {
        QJsonObject json = getBuiltInPreset(name);
        jsonToSettings(json, settings);
        Q_EMIT presetLoaded(name);
        return true;
    }

    // Load from file
    QFile file(presetFilePath(name));
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject())
        return false;

    jsonToSettings(doc.object(), settings);
    Q_EMIT presetLoaded(name);
    return true;
}

bool PresetManager::deletePreset(const QString& name) {
    // Can't delete built-in presets
    if (builtInPresets().contains(name))
        return false;

    QFile file(presetFilePath(name));
    if (file.remove()) {
        Q_EMIT presetDeleted(name);
        return true;
    }
    return false;
}

bool PresetManager::presetExists(const QString& name) const {
    return allPresets().contains(name);
}

QString PresetManager::presetsDirectory() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataPath + "/presets";
}

QString PresetManager::presetFilePath(const QString& name) const {
    return presetsDirectory() + "/" + name + ".json";
}

QJsonObject PresetManager::settingsToJson(const AdjustmentSettings* settings) const {
    QJsonObject json;
    json["exposure"] = settings->exposure();
    json["contrast"] = settings->contrast();
    json["brightness"] = settings->brightness();
    json["highlights"] = settings->highlights();
    json["shadows"] = settings->shadows();
    json["whites"] = settings->whites();
    json["blacks"] = settings->blacks();
    json["temperature"] = settings->temperature();
    json["tint"] = settings->tint();
    json["saturation"] = settings->saturation();
    return json;
}

void PresetManager::jsonToSettings(const QJsonObject& json, AdjustmentSettings* settings) const {
    if (json.contains("exposure"))
        settings->setExposure(json["exposure"].toInt());
    if (json.contains("contrast"))
        settings->setContrast(json["contrast"].toInt());
    if (json.contains("brightness"))
        settings->setBrightness(json["brightness"].toInt());
    if (json.contains("highlights"))
        settings->setHighlights(json["highlights"].toInt());
    if (json.contains("shadows"))
        settings->setShadows(json["shadows"].toInt());
    if (json.contains("whites"))
        settings->setWhites(json["whites"].toInt());
    if (json.contains("blacks"))
        settings->setBlacks(json["blacks"].toInt());
    if (json.contains("temperature"))
        settings->setTemperature(json["temperature"].toInt());
    if (json.contains("tint"))
        settings->setTint(json["tint"].toInt());
    if (json.contains("saturation"))
        settings->setSaturation(json["saturation"].toInt());
}
