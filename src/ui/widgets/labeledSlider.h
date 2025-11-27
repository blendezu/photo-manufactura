#pragma once

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QWidget>

class LabeledSlider : public QWidget {
    Q_OBJECT

   public:
    explicit LabeledSlider(const QString& label, int min, int max, int defaultValue = 0,
                           QWidget* parent = nullptr);

    int value() const;
    void setValue(int value);
    void setRange(int min, int max);
    void reset();

   signals:
    void valueChanged(int value);

   private slots:
    void onSliderChanged(int value);
    void onSpinBoxChanged(int value);

   private:
    QLabel* m_label;
    QSlider* m_slider;
    QSpinBox* m_spinBox;
    int m_defaultValue;
};
