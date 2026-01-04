#ifndef PERSPECTIVE_CROP_H
#define PERSPECTIVE_CROP_H

#include <array>
#include <opencv2/opencv.hpp>

#include "../core/operation_base.h"

/**
 * @brief Four-point perspective transformation and crop
 *
 * Allows selecting 4 arbitrary corner points to perform perspective correction
 * and crop. Points are stored as ratios (0.0 to 1.0) relative to image dimensions
 * for resolution-independent operation.
 *
 * Corner order: [TopLeft, TopRight, BottomRight, BottomLeft]
 */
class PerspectiveCrop : public ImageOperation {
   public:
    /**
     * @brief Corner points as ratios (0.0 to 1.0) relative to image size
     * Order: TopLeft, TopRight, BottomRight, BottomLeft
     */
    struct QuadPoints {
        cv::Point2f topLeft{0.0f, 0.0f};
        cv::Point2f topRight{1.0f, 0.0f};
        cv::Point2f bottomRight{1.0f, 1.0f};
        cv::Point2f bottomLeft{0.0f, 1.0f};

        // Check if this is a simple rectangle (no perspective)
        bool isRectangle() const {
            return topLeft.x == bottomLeft.x && topRight.x == bottomRight.x &&
                   topLeft.y == topRight.y && bottomLeft.y == bottomRight.y;
        }

        // Check if points form a valid convex quadrilateral
        bool isValid() const;

        // Get points as array for OpenCV
        std::array<cv::Point2f, 4> toArray() const {
            return {topLeft, topRight, bottomRight, bottomLeft};
        }

        // Create from array
        static QuadPoints fromArray(const std::array<cv::Point2f, 4>& pts) {
            QuadPoints q;
            q.topLeft = pts[0];
            q.topRight = pts[1];
            q.bottomRight = pts[2];
            q.bottomLeft = pts[3];
            return q;
        }

        // Default rectangle (full image)
        static QuadPoints defaultRect() {
            return QuadPoints();
        }
    };

    /**
     * @brief Output size mode
     */
    enum class OutputMode {
        Auto,        // Calculate output size from quad bounding box
        FixedSize,   // Use specified width/height
        AspectRatio  // Maintain aspect ratio, fit to bounding box
    };

   private:
    QuadPoints m_srcQuad;   // Source quadrilateral (ratio-based)
    cv::Size m_outputSize;  // Output dimensions (0,0 = auto)
    OutputMode m_outputMode = OutputMode::Auto;

   public:
    /**
     * @brief Construct with ratio-based corner points
     * @param quad Four corner points as ratios (0.0-1.0)
     */
    explicit PerspectiveCrop(const QuadPoints& quad) : m_srcQuad(quad) {}

    /**
     * @brief Construct with ratio-based points and fixed output size
     */
    PerspectiveCrop(const QuadPoints& quad, const cv::Size& outputSize)
        : m_srcQuad(quad), m_outputSize(outputSize), m_outputMode(OutputMode::FixedSize) {}

    std::string getName() const override {
        return "PerspectiveCrop";
    }

    // Setters
    void setQuadPoints(const QuadPoints& quad) {
        m_srcQuad = quad;
    }
    void setOutputSize(const cv::Size& size) {
        m_outputSize = size;
        m_outputMode = OutputMode::FixedSize;
    }
    void setOutputMode(OutputMode mode) {
        m_outputMode = mode;
    }

    // Getters
    const QuadPoints& getQuadPoints() const {
        return m_srcQuad;
    }
    cv::Size getOutputSize() const {
        return m_outputSize;
    }
    OutputMode getOutputMode() const {
        return m_outputMode;
    }

    /**
     * @brief Apply perspective transformation
     * @param srcImg Input image
     * @return Perspective-corrected and cropped image
     */
    cv::Mat apply(const cv::Mat& srcImg) override;

    /**
     * @brief Convert ratio-based points to absolute pixel coordinates
     */
    std::array<cv::Point2f, 4> toAbsoluteCoords(int imgWidth, int imgHeight) const;

    /**
     * @brief Calculate the output rectangle size based on the quad
     */
    cv::Size calculateOutputSize(int imgWidth, int imgHeight) const;

   private:
    /**
     * @brief Calculate destination quad (rectangle) for the transform
     */
    std::array<cv::Point2f, 4> calculateDestQuad(const cv::Size& outputSize) const;
};

#endif  // PERSPECTIVE_CROP_H
