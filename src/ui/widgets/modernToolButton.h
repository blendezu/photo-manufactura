#pragma once

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @brief Modern tool button with icon, label, and hover effects
 * Provides a professional, macOS-inspired look for toolbar actions
 */
class ModernToolButton : public QWidget {
    Q_OBJECT

   public:
    explicit ModernToolButton(const QString& text, const QString& iconPath = "",
                              QWidget* parent = nullptr);

    void setIcon(const QString& iconPath);
    void setText(const QString& text);
    void setChecked(bool checked);
    bool isChecked() const {
        return m_checked;
    }
    void setCheckable(bool checkable) {
        m_checkable = checkable;
    }
    void setFlat(bool flat);                 // Transparent background when not hovered/checked
    void setBadgeText(const QString& text);  // For showing status like "AI"

   Q_SIGNALS:
    void clicked();
    void toggled(bool checked);

   protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

   private:
    void updateStyle();

    QPushButton* m_button;
    QLabel* m_label;
    QLabel* m_badge;
    QVBoxLayout* m_layout;
    QGraphicsDropShadowEffect* m_shadowEffect;

    bool m_hovered = false;
    bool m_pressed = false;
    bool m_checked = false;
    bool m_checkable = false;
    bool m_flat = false;
    QString m_iconPath;
};

/**
 * @brief Filter preview card showing a thumbnail preview of the filter effect
 */
class FilterPreviewCard : public QWidget {
    Q_OBJECT

   public:
    explicit FilterPreviewCard(const QString& name, const QString& previewPath = "",
                               QWidget* parent = nullptr);

    void setPreviewImage(const QImage& preview);
    void setSelected(bool selected);
    bool isSelected() const {
        return m_selected;
    }
    QString filterName() const {
        return m_name;
    }

   Q_SIGNALS:
    void clicked(const QString& filterName);

   protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

   private:
    void updateStyle();

    QString m_name;
    QLabel* m_previewLabel;
    QLabel* m_nameLabel;
    bool m_hovered = false;
    bool m_selected = false;
};

/**
 * @brief Container for filter preview cards with horizontal scrolling
 */
class FilterGalleryWidget : public QWidget {
    Q_OBJECT

   public:
    explicit FilterGalleryWidget(QWidget* parent = nullptr);

    void addFilter(const QString& name, const QString& previewPath = "");
    void clearFilters();
    void setSelectedFilter(const QString& name);
    QString selectedFilter() const {
        return m_selectedFilter;
    }

   Q_SIGNALS:
    void filterSelected(const QString& filterName);

   private:
    QHBoxLayout* m_layout;
    QList<FilterPreviewCard*> m_cards;
    QString m_selectedFilter;
};
