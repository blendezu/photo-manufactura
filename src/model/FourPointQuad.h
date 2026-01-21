#pragma once

#include <QPointF>

/**
 * @brief Four corner points as ratios (0.0 to 1.0) for perspective crop
 *
 * Each point represents a corner that can be dragged independently.
 * Order: TopLeft, TopRight, BottomRight, BottomLeft
 *
 * This struct is shared between UI and Model layers.
 */
struct FourPointQuad {
    QPointF topLeft{0.0, 0.0};
    QPointF topRight{1.0, 0.0};
    QPointF bottomRight{1.0, 1.0};
    QPointF bottomLeft{0.0, 1.0};

    // Get point by index (0=TL, 1=TR, 2=BR, 3=BL)
    QPointF& operator[](int index) {
        switch (index) {
            case 0:
                return topLeft;
            case 1:
                return topRight;
            case 2:
                return bottomRight;
            case 3:
                return bottomLeft;
            default:
                return topLeft;
        }
    }

    const QPointF& operator[](int index) const {
        switch (index) {
            case 0:
                return topLeft;
            case 1:
                return topRight;
            case 2:
                return bottomRight;
            case 3:
                return bottomLeft;
            default:
                return topLeft;
        }
    }

    // Check if this is a simple rectangle
    bool isRectangle() const {
        return qFuzzyCompare(topLeft.x(), bottomLeft.x()) &&
               qFuzzyCompare(topRight.x(), bottomRight.x()) &&
               qFuzzyCompare(topLeft.y(), topRight.y()) &&
               qFuzzyCompare(bottomLeft.y(), bottomRight.y());
    }

    // Reset to full image rectangle
    void reset() {
        topLeft = QPointF(0.0, 0.0);
        topRight = QPointF(1.0, 0.0);
        bottomRight = QPointF(1.0, 1.0);
        bottomLeft = QPointF(0.0, 1.0);
    }
};
