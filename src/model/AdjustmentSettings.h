#pragma once

#include <QObject>

/**
 * @brief Holds all image adjustment settings
 *
 * Model class that stores the current adjustment values.
 * These values are applied to the image via the image processing pipeline.
 */
class AdjustmentSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(int exposure READ exposure WRITE setExposure NOTIFY exposureChanged)
    Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(int highlights READ highlights WRITE setHighlights NOTIFY highlightsChanged)
    Q_PROPERTY(int shadows READ shadows WRITE setShadows NOTIFY shadowsChanged)
    Q_PROPERTY(int whites READ whites WRITE setWhites NOTIFY whitesChanged)
    Q_PROPERTY(int blacks READ blacks WRITE setBlacks NOTIFY blacksChanged)
    Q_PROPERTY(int temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(int tint READ tint WRITE setTint NOTIFY tintChanged)
    Q_PROPERTY(int saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)

   public:
    explicit AdjustmentSettings(QObject* parent = nullptr);
    ~AdjustmentSettings() = default;

    // Basic adjustments
    int exposure() const {
        return m_exposure;
    }
    int contrast() const {
        return m_contrast;
    }
    int brightness() const {
        return m_brightness;
    }
    int highlights() const {
        return m_highlights;
    }
    int shadows() const {
        return m_shadows;
    }
    int whites() const {
        return m_whites;
    }
    int blacks() const {
        return m_blacks;
    }

    // Color adjustments
    int temperature() const {
        return m_temperature;
    }
    int tint() const {
        return m_tint;
    }
    int saturation() const {
        return m_saturation;
    }

    // Check if any adjustment is non-default
    bool hasAdjustments() const;

   public Q_SLOTS:
    // Basic adjustments
    void setExposure(int value);
    void setContrast(int value);
    void setBrightness(int value);
    void setHighlights(int value);
    void setShadows(int value);
    void setWhites(int value);
    void setBlacks(int value);

    // Color adjustments
    void setTemperature(int value);
    void setTint(int value);
    void setSaturation(int value);

    // Reset all to defaults
    void resetAll();

Q_SIGNALS:
    void exposureChanged(int value);
    void contrastChanged(int value);
    void brightnessChanged(int value);
    void highlightsChanged(int value);
    void shadowsChanged(int value);
    void whitesChanged(int value);
    void blacksChanged(int value);
    void temperatureChanged(int value);
    void tintChanged(int value);
    void saturationChanged(int value);
    void settingsReset();
    void anySettingChanged();

   private:
    // Basic adjustments (range: -100 to +100, default: 0)
    int m_exposure = 0;
    int m_contrast = 0;
    int m_brightness = 0;
    int m_highlights = 0;
    int m_shadows = 0;
    int m_whites = 0;
    int m_blacks = 0;

    // Color adjustments (range: -100 to +100, default: 0)
    int m_temperature = 0;
    int m_tint = 0;
    int m_saturation = 0;
};
