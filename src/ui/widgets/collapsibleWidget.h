#pragma once

#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class CollapsibleWidget : public QWidget {
    Q_OBJECT

   public:
    explicit CollapsibleWidget(const QString& title = "", QWidget* parent = nullptr);
    void setContentLayout(QLayout* contentLayout);

   private slots:
    void toggle(bool collapsed);

   private:
    QGridLayout* m_mainLayout;
    QToolButton* m_toggleButton;
    QFrame* m_headerLine;
    QParallelAnimationGroup* m_toggleAnimation;
    QScrollArea* m_contentArea;
    int m_animationDuration{300};
};
