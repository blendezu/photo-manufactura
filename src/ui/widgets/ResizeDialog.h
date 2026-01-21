#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>

/**
 * @brief Dialog for resizing images
 * 
 * Allows user to specify new dimensions with aspect ratio lock option.
 */
class ResizeDialog : public QDialog {
    Q_OBJECT

public:
    explicit ResizeDialog(int currentWidth, int currentHeight, QWidget* parent = nullptr);
    ~ResizeDialog() = default;

    int newWidth() const;
    int newHeight() const;

private slots:
    void onWidthChanged(int value);
    void onHeightChanged(int value);
    void onLockAspectChanged(bool checked);

private:
    void setupUI();
    void updateFromWidth();
    void updateFromHeight();

    QSpinBox* m_widthSpin;
    QSpinBox* m_heightSpin;
    QCheckBox* m_lockAspectCheck;
    QLabel* m_previewLabel;

    int m_originalWidth;
    int m_originalHeight;
    double m_aspectRatio;
    bool m_updating = false;  // Prevent recursive updates
};
