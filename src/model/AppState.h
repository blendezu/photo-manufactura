#pragma once

#include <QObject>
#include <QString>

/**
 * @brief Holds application-level state
 *
 * Model class that stores UI state such as theme, panel visibility,
 * zoom levels, and other application-wide settings.
 */
class AppState : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(double zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(bool histogramVisible READ histogramVisible WRITE setHistogramVisible NOTIFY
                   histogramVisibleChanged)
    Q_PROPERTY(bool toolPanelVisible READ toolPanelVisible WRITE setToolPanelVisible NOTIFY
                   toolPanelVisibleChanged)
    Q_PROPERTY(bool adjustmentPanelVisible READ adjustmentPanelVisible WRITE
                   setAdjustmentPanelVisible NOTIFY adjustmentPanelVisibleChanged)

   public:
    explicit AppState(QObject* parent = nullptr);
    ~AppState() = default;

    // Theme
    QString theme() const {
        return m_theme;
    }

    // Zoom
    double zoomLevel() const {
        return m_zoomLevel;
    }
    static constexpr double MinZoom = 0.1;
    static constexpr double MaxZoom = 10.0;
    static constexpr double DefaultZoom = 1.0;

    // Panel visibility
    bool histogramVisible() const {
        return m_histogramVisible;
    }
    bool toolPanelVisible() const {
        return m_toolPanelVisible;
    }
    bool adjustmentPanelVisible() const {
        return m_adjustmentPanelVisible;
    }

   public Q_SLOTS:
    void setTheme(const QString& theme);
    void setZoomLevel(double level);
    void setHistogramVisible(bool visible);
    void setToolPanelVisible(bool visible);
    void setAdjustmentPanelVisible(bool visible);

    // Zoom helpers
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void zoomToActual();

    // Zoom persistence per document
    void saveZoomForFile(const QString& filePath);
    void restoreZoomForFile(const QString& filePath);

    // Toggle helpers
    void toggleHistogram();
    void toggleToolPanel();
    void toggleAdjustmentPanel();

Q_SIGNALS:
    void themeChanged(const QString& theme);
    void zoomLevelChanged(double level);
    void histogramVisibleChanged(bool visible);
    void toolPanelVisibleChanged(bool visible);
    void adjustmentPanelVisibleChanged(bool visible);

   private:
    QString m_theme = "dark";
    double m_zoomLevel = 1.0;
    bool m_histogramVisible = true;
    bool m_toolPanelVisible = true;
    bool m_adjustmentPanelVisible = true;
};
