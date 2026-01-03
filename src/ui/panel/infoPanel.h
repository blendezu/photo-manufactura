#pragma once

#include <QImage>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class HistogramWidget;

class InfoPanel : public QWidget {
    Q_OBJECT
   public:
    explicit InfoPanel(QWidget* parent = nullptr);
    ~InfoPanel();

    void updateImageInfo(const QString& filePath, int width, int height, const QString& format);
    void updateHistogram(const QImage& image);
    void clearInfo();
    void updateFileSize(qint64 sizeBytes);
    void updateMouseCoords(const QPoint& widgetPos, const QPoint& imagePos);

   private:
    void setupUI();

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_filePathLabel;
    QLabel* m_dimensionsLabel;
    QLabel* m_formatLabel;
    QLabel* m_fileSizeLabel;
    QLabel* m_colorSpaceLabel;
    QLabel* m_megapixelsLabel;
    QLabel* m_mouseCoordsLabel;
    HistogramWidget* m_histogramWidget;
};