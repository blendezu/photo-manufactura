#pragma once

#include <QImage>
#include <QPainter>
#include <QWidget>
#include <array>

/**
 * @brief Widget that displays RGB histogram of an image
 *
 * Shows the distribution of pixel values for Red, Green, Blue channels
 * and an optional Luminance overlay.
 */
class HistogramWidget : public QWidget {
    Q_OBJECT

   public:
    explicit HistogramWidget(QWidget* parent = nullptr);
    ~HistogramWidget() override = default;

    void setImage(const QImage& image);
    void clear();

    // Display options
    void setShowRed(bool show);
    void setShowGreen(bool show);
    void setShowBlue(bool show);
    void setShowLuminance(bool show);

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    void calculateHistogram(const QImage& image);
    void drawChannel(QPainter& painter, const std::array<int, 256>& data, const QColor& color,
                     int maxValue);

    // Histogram data for each channel (256 bins)
    std::array<int, 256> m_redHist{};
    std::array<int, 256> m_greenHist{};
    std::array<int, 256> m_blueHist{};
    std::array<int, 256> m_luminanceHist{};

    // Maximum value for scaling
    int m_maxValue = 0;

    // Display flags
    bool m_showRed = true;
    bool m_showGreen = true;
    bool m_showBlue = true;
    bool m_showLuminance = false;

    // State
    bool m_hasData = false;
};
