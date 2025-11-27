#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class InfoPanel : public QWidget {
    Q_OBJECT
   public:
    explicit InfoPanel(QWidget* parent = nullptr);
    ~InfoPanel();

    void updateImageInfo(const QString& filePath, int width, int height, const QString& format);
    void clearInfo();

   private:
    void setupUI();

    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_filePathLabel;
    QLabel* m_dimensionsLabel;
    QLabel* m_formatLabel;
    QLabel* m_histogramPlaceholder;
};