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
        emit exposureChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setContrast(int value) {
    if (m_contrast != value) {
        m_contrast = value;
        emit contrastChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setBrightness(int value) {
    if (m_brightness != value) {
        m_brightness = value;
        emit brightnessChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setHighlights(int value) {
    if (m_highlights != value) {
        m_highlights = value;
        emit highlightsChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setShadows(int value) {
    if (m_shadows != value) {
        m_shadows = value;
        emit shadowsChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setWhites(int value) {
    if (m_whites != value) {
        m_whites = value;
        emit whitesChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setBlacks(int value) {
    if (m_blacks != value) {
        m_blacks = value;
        emit blacksChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setTemperature(int value) {
    if (m_temperature != value) {
        m_temperature = value;
        emit temperatureChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setTint(int value) {
    if (m_tint != value) {
        m_tint = value;
        emit tintChanged(value);
        emit anySettingChanged();
    }
}

void AdjustmentSettings::setSaturation(int value) {
    if (m_saturation != value) {
        m_saturation = value;
        emit saturationChanged(value);
        emit anySettingChanged();
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
    emit settingsReset();
    emit anySettingChanged();
}
