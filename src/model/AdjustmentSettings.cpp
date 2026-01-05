#include "AdjustmentSettings.h"

AdjustmentSettings::AdjustmentSettings(QObject* parent) : QObject(parent) {}

bool AdjustmentSettings::hasAdjustments() const {
    return m_exposure != 0 || m_contrast != 0 || m_brightness != 0 || m_highlights != 0 ||
           m_shadows != 0 || m_whites != 0 || m_blacks != 0 || m_temperature != 0 || m_tint != 0 ||
           m_saturation != 0 || m_denoise != 0 || m_clarity != 0 || m_sharpening != 0;
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

void AdjustmentSettings::setDenoise(int value) {
    if (m_denoise != value) {
        m_denoise = value;
        Q_EMIT denoiseChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setClarity(int value) {
    if (m_clarity != value) {
        m_clarity = value;
        Q_EMIT clarityChanged(value);
        Q_EMIT anySettingChanged();
    }
}

void AdjustmentSettings::setSharpening(int value) {
    if (m_sharpening != value) {
        m_sharpening = value;
        Q_EMIT sharpeningChanged(value);
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
    m_denoise = 0;
    m_clarity = 0;
    m_sharpening = 0;
    Q_EMIT settingsReset();
    Q_EMIT anySettingChanged();
}

AdjustmentSettings::Snapshot AdjustmentSettings::createSnapshot() const {
    Snapshot s;
    s.exposure = m_exposure;
    s.contrast = m_contrast;
    s.brightness = m_brightness;
    s.highlights = m_highlights;
    s.shadows = m_shadows;
    s.whites = m_whites;
    s.blacks = m_blacks;
    s.temperature = m_temperature;
    s.tint = m_tint;
    s.saturation = m_saturation;
    s.denoise = m_denoise;
    s.clarity = m_clarity;
    s.sharpening = m_sharpening;
    return s;
}

void AdjustmentSettings::applySnapshot(const Snapshot& s) {
    if (createSnapshot() == s)
        return;

    // Apply basic adjustments
    if (m_exposure != s.exposure)
        setExposure(s.exposure);
    if (m_contrast != s.contrast)
        setContrast(s.contrast);
    if (m_brightness != s.brightness)
        setBrightness(s.brightness);
    if (m_highlights != s.highlights)
        setHighlights(s.highlights);
    if (m_shadows != s.shadows)
        setShadows(s.shadows);
    if (m_whites != s.whites)
        setWhites(s.whites);
    if (m_blacks != s.blacks)
        setBlacks(s.blacks);

    // Apply color adjustments
    if (m_temperature != s.temperature)
        setTemperature(s.temperature);
    if (m_tint != s.tint)
        setTint(s.tint);
    if (m_saturation != s.saturation)
        setSaturation(s.saturation);

    // Apply detail adjustments
    if (m_denoise != s.denoise)
        setDenoise(s.denoise);
    if (m_clarity != s.clarity)
        setClarity(s.clarity);
    if (m_sharpening != s.sharpening)
        setSharpening(s.sharpening);
}
