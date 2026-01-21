#include "histogramWidget.h"

#include <QPainterPath>
#include <algorithm>
#include <cmath>

HistogramWidget::HistogramWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(120);
    setMinimumWidth(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Dark background for the histogram
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(30, 30, 30));
    setPalette(pal);
}

void HistogramWidget::setImage(const QImage& image) {
    if (image.isNull()) {
        clear();
        return;
    }

    calculateHistogram(image);
    m_hasData = true;
    update();
}

void HistogramWidget::clear() {
    m_redHist.fill(0);
    m_greenHist.fill(0);
    m_blueHist.fill(0);
    m_luminanceHist.fill(0);
    m_maxValue = 0;
    m_hasData = false;
    update();
}

void HistogramWidget::setShowRed(bool show) {
    m_showRed = show;
    update();
}

void HistogramWidget::setShowGreen(bool show) {
    m_showGreen = show;
    update();
}

void HistogramWidget::setShowBlue(bool show) {
    m_showBlue = show;
    update();
}

void HistogramWidget::setShowLuminance(bool show) {
    m_showLuminance = show;
    update();
}

void HistogramWidget::calculateHistogram(const QImage& image) {
    // Reset histograms
    m_redHist.fill(0);
    m_greenHist.fill(0);
    m_blueHist.fill(0);
    m_luminanceHist.fill(0);

    // Convert to RGB format for consistent processing
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

    const int width = rgbImage.width();
    const int height = rgbImage.height();

    // Calculate histograms
    for (int y = 0; y < height; ++y) {
        const uchar* scanLine = rgbImage.constScanLine(y);
        for (int x = 0; x < width; ++x) {
            int r = scanLine[x * 3];
            int g = scanLine[x * 3 + 1];
            int b = scanLine[x * 3 + 2];

            m_redHist[r]++;
            m_greenHist[g]++;
            m_blueHist[b]++;

            // Calculate luminance (ITU-R BT.709)
            int lum = static_cast<int>(0.2126 * r + 0.7152 * g + 0.0722 * b);
            lum = std::clamp(lum, 0, 255);
            m_luminanceHist[lum]++;
        }
    }

    // Find maximum value for scaling (exclude extreme ends which can be spikes)
    m_maxValue = 0;
    for (int i = 5; i < 251; ++i) {
        m_maxValue = std::max(m_maxValue, m_redHist[i]);
        m_maxValue = std::max(m_maxValue, m_greenHist[i]);
        m_maxValue = std::max(m_maxValue, m_blueHist[i]);
    }

    // Ensure we have a valid max value
    if (m_maxValue == 0) {
        for (int i = 0; i < 256; ++i) {
            m_maxValue = std::max(m_maxValue, m_redHist[i]);
            m_maxValue = std::max(m_maxValue, m_greenHist[i]);
            m_maxValue = std::max(m_maxValue, m_blueHist[i]);
        }
    }
}

void HistogramWidget::drawChannel(QPainter& painter, const std::array<int, 256>& data,
                                  const QColor& color, int maxValue) {
    if (maxValue <= 0)
        return;

    const int w = width() - 20;  // Padding
    const int h = height() - 20;
    const int offsetX = 10;
    const int offsetY = 10;

    // Create path for filled histogram
    QPainterPath path;
    path.moveTo(offsetX, offsetY + h);

    for (int i = 0; i < 256; ++i) {
        double x = offsetX + (i * w) / 255.0;
        double normalizedValue = static_cast<double>(data[i]) / maxValue;
        // Use logarithmic scale for better visualization
        normalizedValue = std::log1p(normalizedValue * 100) / std::log1p(100);
        double y = offsetY + h - (normalizedValue * h);

        if (i == 0) {
            path.lineTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    path.lineTo(offsetX + w, offsetY + h);
    path.closeSubpath();

    // Draw filled area with transparency
    QColor fillColor = color;
    fillColor.setAlpha(80);
    painter.fillPath(path, fillColor);

    // Draw outline
    painter.setPen(QPen(color, 1));

    // Draw the line on top
    for (int i = 0; i < 255; ++i) {
        double x1 = offsetX + (i * w) / 255.0;
        double x2 = offsetX + ((i + 1) * w) / 255.0;

        double nv1 = static_cast<double>(data[i]) / maxValue;
        double nv2 = static_cast<double>(data[i + 1]) / maxValue;
        nv1 = std::log1p(nv1 * 100) / std::log1p(100);
        nv2 = std::log1p(nv2 * 100) / std::log1p(100);

        double y1 = offsetY + h - (nv1 * h);
        double y2 = offsetY + h - (nv2 * h);

        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }
}

void HistogramWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    painter.fillRect(rect(), QColor(30, 30, 30));

    if (!m_hasData) {
        // Draw placeholder text
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(rect(), Qt::AlignCenter, tr("No image loaded"));
        return;
    }

    // Draw grid lines
    painter.setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));
    const int w = width() - 20;
    const int h = height() - 20;
    const int offsetX = 10;
    const int offsetY = 10;

    // Vertical grid lines (at 0, 64, 128, 192, 255)
    for (int i = 0; i <= 4; ++i) {
        int x = offsetX + (i * w) / 4;
        painter.drawLine(x, offsetY, x, offsetY + h);
    }

    // Horizontal grid lines
    for (int i = 0; i <= 4; ++i) {
        int y = offsetY + (i * h) / 4;
        painter.drawLine(offsetX, y, offsetX + w, y);
    }

    // Enable blending for overlapping channels
    painter.setCompositionMode(QPainter::CompositionMode_Plus);

    // Draw channels (order matters for visibility)
    if (m_showBlue) {
        drawChannel(painter, m_blueHist, QColor(50, 100, 255), m_maxValue);
    }
    if (m_showGreen) {
        drawChannel(painter, m_greenHist, QColor(50, 255, 100), m_maxValue);
    }
    if (m_showRed) {
        drawChannel(painter, m_redHist, QColor(255, 80, 80), m_maxValue);
    }

    // Reset composition mode for luminance
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    if (m_showLuminance) {
        drawChannel(painter, m_luminanceHist, QColor(200, 200, 200), m_maxValue);
    }

    // Draw border
    painter.setPen(QPen(QColor(80, 80, 80), 1));
    painter.drawRect(offsetX, offsetY, w, h);
}
