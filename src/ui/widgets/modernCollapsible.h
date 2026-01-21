#pragma once

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @brief Modern collapsible section widget with smooth animations.
 *
 * Features:
 * - Animated expand/collapse with easing
 * - Modern header styling with icon
 * - Optional badge/count indicator
 * - Subtle hover effects
 */
class ModernCollapsible : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight)

   public:
    explicit ModernCollapsible(const QString& title, const QString& icon = "",
                               QWidget* parent = nullptr);
    ~ModernCollapsible() override = default;

    void setContentLayout(QLayout* contentLayout);
    void setExpanded(bool expanded);
    bool isExpanded() const {
        return m_expanded;
    }
    void setBadgeCount(int count);
    void setIcon(const QString& icon);

    int contentHeight() const {
        return m_currentContentHeight;
    }
    void setContentHeight(int height);

   Q_SIGNALS:
    void expandedChanged(bool expanded);

   private Q_SLOTS:
    void toggle();

   protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool event(QEvent* event) override;

   private:
    void setupUI();
    void updateHeaderStyle(bool hovered);
    void updateChevron();

    QString m_title;
    QString m_iconText;
    bool m_expanded;
    int m_currentContentHeight;
    int m_targetContentHeight;

    // UI Components
    QWidget* m_headerWidget;
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_badgeLabel;
    QLabel* m_chevronLabel;
    QWidget* m_contentContainer;
    QVBoxLayout* m_contentLayout;

    // Animation
    QPropertyAnimation* m_animation;
    int m_animationDuration;
};

/**
 * @brief Section header for grouping controls without collapse functionality.
 * Used for simple visual separation.
 */
class SectionHeader : public QWidget {
    Q_OBJECT

   public:
    explicit SectionHeader(const QString& title, const QString& icon = "",
                           QWidget* parent = nullptr);

   private:
    void setupUI();
    QString m_title;
    QString m_icon;
};
