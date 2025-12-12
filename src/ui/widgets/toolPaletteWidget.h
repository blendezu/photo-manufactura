#pragma once

#include <QVBoxLayout>
#include <QWidget>

class ToolPaletteWidget : public QWidget {
    Q_OBJECT

   public:
    explicit ToolPaletteWidget(const QString& title = "", QWidget* parent = nullptr);
    void addToolButton(const QString& toolName, const QString& iconPath);

   private slots:
    bool isToolButtonChecked(const QString& toolName);
    void setToolButtonChecked(const QString& toolName, bool checked);
    
    void onToolButtonClicked();

   private:
    QVBoxLayout* m_layout;
};