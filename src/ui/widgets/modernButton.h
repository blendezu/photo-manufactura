#pragma once

#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QWidget>

/**
 * @brief Modern styled button with various visual styles.
 *
 * Styles:
 * - Primary: Blue accent color (main action)
 * - Secondary: Subtle gray (secondary action)
 * - Ghost: Transparent with border (tertiary action)
 * - Danger: Red accent (destructive action)
 * - Success: Green accent (positive action)
 */
class ModernButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)

   public:
    enum Style { Primary, Secondary, Ghost, Danger, Success };
    Q_ENUM(Style)

    enum Size { Small, Medium, Large };
    Q_ENUM(Size)

    explicit ModernButton(const QString& text, Style style = Secondary, QWidget* parent = nullptr);

    void setButtonStyle(Style style);
    void setButtonSize(Size size);
    void setIcon(const QString& iconPath);
    void setIconEmoji(const QString& emoji);
    void setLoading(bool loading);

    qreal hoverProgress() const {
        return m_hoverProgress;
    }
    void setHoverProgress(qreal progress);

   protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

   private:
    void updateStyle();

    Style m_style;
    Size m_size;
    QString m_emoji;
    bool m_loading;
    qreal m_hoverProgress;
    QPropertyAnimation* m_hoverAnimation;
};

/**
 * @brief Icon-only button for toolbar use.
 */
class IconButton : public QPushButton {
    Q_OBJECT

   public:
    explicit IconButton(const QString& iconPath, const QString& tooltip = "",
                        QWidget* parent = nullptr);
    void setIconEmoji(const QString& emoji);
    void setActive(bool active);

   protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

   private:
    void updateStyle(bool hovered);
    bool m_active;
};

/**
 * @brief Toggle button that switches between two states.
 */
class ToggleButton : public QWidget {
    Q_OBJECT

   public:
    explicit ToggleButton(const QString& labelOff, const QString& labelOn,
                          QWidget* parent = nullptr);

    bool isChecked() const {
        return m_checked;
    }
    void setChecked(bool checked);

   Q_SIGNALS:
    void toggled(bool checked);

   protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

   private:
    QString m_labelOff;
    QString m_labelOn;
    bool m_checked;
    QPropertyAnimation* m_animation;
    qreal m_animationProgress;
};
