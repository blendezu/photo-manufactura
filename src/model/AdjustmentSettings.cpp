#include "AdjustmentSettings.h"

AdjustmentSettings::AdjustmentSettings(QObject* parent) : QObject(parent) {}

bool AdjustmentSettings::hasAdjustments() const {
    return m_exposure != 0 || m_contrast != 0 || m_brightness != 0 || m_highlights != 0 ||
           m_shadows != 0 || m_whites != 0 || m_blacks != 0 || m_temperature != 0 || m_tint != 0 ||
           m_saturation != 0;
}

void AdjustmentSettings::setExposure(int value) {
    if (m_exposure != value) {
        m_exposure = value;
        Q_EMIT exposureChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setContrast(int value) {
    if (m_contrast != value) {
        m_contrast = value;
        Q_EMIT contrastChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setBrightness(int value) {
    if (m_brightness != value) {
        m_brightness = value;
        Q_EMIT brightnessChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setHighlights(int value) {
    if (m_highlights != value) {
        m_highlights = value;
        Q_EMIT highlightsChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setShadows(int value) {
    if (m_shadows != value) {
        m_shadows = value;
        Q_EMIT shadowsChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setWhites(int value) {
    if (m_whites != value) {
        m_whites = value;
        Q_EMIT whitesChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setBlacks(int value) {
    if (m_blacks != value) {
        m_blacks = value;
        Q_EMIT blacksChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setTemperature(int value) {
    if (m_temperature != value) {
        m_temperature = value;
        Q_EMIT temperatureChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setTint(int value) {
    if (m_tint != value) {
        m_tint = value;
        Q_EMIT tintChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setSaturation(int value) {
    if (m_saturation != value) {
        m_saturation = value;
        Q_EMIT saturationChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::resetAll() {
    m_exposure = 0;
    m_contrast = 0;
    m_brightness = 0;
    m_highlights = 0;
    m_shadows = 0;
    m_whites = 0;
    m_blacks = 0;
    m_temperature = 0;
    m_tint = 0;
    m_saturation = 0;
    Q_EMIT settingsReset();
    Q_EMIT anySettingChanged();
}
