#pragma once

#include <QtOpenGLWidgets/QtOpenGLWidgets>

class CanvasWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
   public:
    // Constructor and Destructor
    explicit CanvasWidget(QWidget* parent = nullptr);
    ~CanvasWidget() override;

   signals:

    // Overridden methods from QOpenGLWidget
   protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

   private:
    // Add private members here as needed
};