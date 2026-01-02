#pragma once

#include <QHBoxLayout>
#include <QWidget>

class ToolPaletteWidget : public QWidget {
    Q_OBJECT

   public:
    explicit ToolPaletteWidget(const QString& title = "", QWidget* parent = nullptr);
    void addToolButton(const QString& toolName, const QString& iconPath);

   Q_SIGNALS:
    void toolActivated(const QString& toolName);

   private Q_SLOTS:
    bool isToolButtonChecked(const QString& toolName);
    void setToolButtonChecked(const QString& toolName, bool checked);

    void onToolButtonClicked();

   private:
    QHBoxLayout* m_layout;
};