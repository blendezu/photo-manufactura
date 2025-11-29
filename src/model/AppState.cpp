#include "AppState.h"

#include <algorithm>

AppState::AppState(QObject* parent) : QObject(parent) {}

void AppState::setTheme(const QString& theme) {
    if (m_theme != theme) {
        m_theme = theme;
        emit themeChanged(theme);
    }
}

void AppState::setZoomLevel(double level) {
    // Clamp to valid range
    double clampedLevel = std::clamp(level, MinZoom, MaxZoom);
    if (m_zoomLevel != clampedLevel) {
        m_zoomLevel = clampedLevel;
        emit zoomLevelChanged(clampedLevel);
    }
}

void AppState::setHistogramVisible(bool visible) {
    if (m_histogramVisible != visible) {
        m_histogramVisible = visible;
        emit histogramVisibleChanged(visible);
    }
}

void AppState::setToolPanelVisible(bool visible) {
    if (m_toolPanelVisible != visible) {
        m_toolPanelVisible = visible;
        emit toolPanelVisibleChanged(visible);
    }
}

void AppState::setAdjustmentPanelVisible(bool visible) {
    if (m_adjustmentPanelVisible != visible) {
        m_adjustmentPanelVisible = visible;
        emit adjustmentPanelVisibleChanged(visible);
    }
}

void AppState::zoomIn() {
    // Increase by 25%
    setZoomLevel(m_zoomLevel * 1.25);
}

void AppState::zoomOut() {
    // Decrease by 25%
    setZoomLevel(m_zoomLevel / 1.25);
}

void AppState::zoomToFit() {
    // This would normally need image dimensions vs viewport
    // For now, just reset to default
    setZoomLevel(DefaultZoom);
}

void AppState::zoomToActual() {
    setZoomLevel(1.0);
}

void AppState::toggleHistogram() {
    setHistogramVisible(!m_histogramVisible);
}

void AppState::toggleToolPanel() {
    setToolPanelVisible(!m_toolPanelVisible);
}

void AppState::toggleAdjustmentPanel() {
    setAdjustmentPanelVisible(!m_adjustmentPanelVisible);
}
