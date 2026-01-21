#include "perspective_crop.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

// Check if the quad forms a valid convex quadrilateral
bool PerspectiveCrop::QuadPoints::isValid() const {
    // All points must be within [0, 1] range
    auto inRange = [](const cv::Point2f& p) {
        return p.x >= 0.0f && p.x <= 1.0f && p.y >= 0.0f && p.y <= 1.0f;
    };

    if (!inRange(topLeft) || !inRange(topRight) || !inRange(bottomRight) || !inRange(bottomLeft)) {
        return false;
    }

    // Check for convexity using cross product
    // For a convex polygon, all cross products of consecutive edges should have same sign
    auto cross = [](const cv::Point2f& o, const cv::Point2f& a, const cv::Point2f& b) {
        return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
    };

    float c1 = cross(topLeft, topRight, bottomRight);
    float c2 = cross(topRight, bottomRight, bottomLeft);
    float c3 = cross(bottomRight, bottomLeft, topLeft);
    float c4 = cross(bottomLeft, topLeft, topRight);

    // All should be same sign (all positive or all negative)
    bool allPositive = c1 > 0 && c2 > 0 && c3 > 0 && c4 > 0;
    bool allNegative = c1 < 0 && c2 < 0 && c3 < 0 && c4 < 0;

    return allPositive || allNegative;
}

std::array<cv::Point2f, 4> PerspectiveCrop::toAbsoluteCoords(int imgWidth, int imgHeight) const {
    float w = static_cast<float>(imgWidth);
    float h = static_cast<float>(imgHeight);

    return {cv::Point2f(m_srcQuad.topLeft.x * w, m_srcQuad.topLeft.y * h),
            cv::Point2f(m_srcQuad.topRight.x * w, m_srcQuad.topRight.y * h),
            cv::Point2f(m_srcQuad.bottomRight.x * w, m_srcQuad.bottomRight.y * h),
            cv::Point2f(m_srcQuad.bottomLeft.x * w, m_srcQuad.bottomLeft.y * h)};
}

cv::Size PerspectiveCrop::calculateOutputSize(int imgWidth, int imgHeight) const {
    if (m_outputMode == OutputMode::FixedSize && m_outputSize.width > 0 &&
        m_outputSize.height > 0) {
        return m_outputSize;
    }

    // Get absolute coordinates
    auto absCoords = toAbsoluteCoords(imgWidth, imgHeight);

    // Calculate the width as average of top and bottom edge lengths
    float topWidth = std::sqrt(std::pow(absCoords[1].x - absCoords[0].x, 2) +
                               std::pow(absCoords[1].y - absCoords[0].y, 2));
    float bottomWidth = std::sqrt(std::pow(absCoords[2].x - absCoords[3].x, 2) +
                                  std::pow(absCoords[2].y - absCoords[3].y, 2));

    // Calculate the height as average of left and right edge lengths
    float leftHeight = std::sqrt(std::pow(absCoords[3].x - absCoords[0].x, 2) +
                                 std::pow(absCoords[3].y - absCoords[0].y, 2));
    float rightHeight = std::sqrt(std::pow(absCoords[2].x - absCoords[1].x, 2) +
                                  std::pow(absCoords[2].y - absCoords[1].y, 2));

    int outWidth = static_cast<int>(std::max(topWidth, bottomWidth));
    int outHeight = static_cast<int>(std::max(leftHeight, rightHeight));

    // Ensure minimum size
    outWidth = std::max(outWidth, 1);
    outHeight = std::max(outHeight, 1);

    return cv::Size(outWidth, outHeight);
}

std::array<cv::Point2f, 4> PerspectiveCrop::calculateDestQuad(const cv::Size& outputSize) const {
    float w = static_cast<float>(outputSize.width);
    float h = static_cast<float>(outputSize.height);

    // Destination is always a rectangle
    return {
        cv::Point2f(0, 0),  // Top-left
        cv::Point2f(w, 0),  // Top-right
        cv::Point2f(w, h),  // Bottom-right
        cv::Point2f(0, h)   // Bottom-left
    };
}

cv::Mat PerspectiveCrop::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        throw std::invalid_argument("PerspectiveCrop: Input image is empty");
    }

    if (!m_srcQuad.isValid()) {
        throw std::invalid_argument("PerspectiveCrop: Invalid quadrilateral points");
    }

    // Get source points in absolute coordinates
    auto srcPoints = toAbsoluteCoords(srcImg.cols, srcImg.rows);

    // If it's a simple rectangle, use regular crop for efficiency
    if (m_srcQuad.isRectangle()) {
        int x = static_cast<int>(srcPoints[0].x);
        int y = static_cast<int>(srcPoints[0].y);
        int w = static_cast<int>(srcPoints[1].x - srcPoints[0].x);
        int h = static_cast<int>(srcPoints[3].y - srcPoints[0].y);

        // Clamp to image bounds
        x = std::max(0, std::min(x, srcImg.cols - 1));
        y = std::max(0, std::min(y, srcImg.rows - 1));
        w = std::min(w, srcImg.cols - x);
        h = std::min(h, srcImg.rows - y);

        if (w <= 0 || h <= 0) {
            throw std::invalid_argument("PerspectiveCrop: Invalid crop region");
        }

        return srcImg(cv::Rect(x, y, w, h)).clone();
    }

    // Calculate output size
    cv::Size outSize = calculateOutputSize(srcImg.cols, srcImg.rows);

    // Get destination points (rectangle)
    auto dstPoints = calculateDestQuad(outSize);

    // Create point vectors for OpenCV
    std::vector<cv::Point2f> srcPts(srcPoints.begin(), srcPoints.end());
    std::vector<cv::Point2f> dstPts(dstPoints.begin(), dstPoints.end());

    // Calculate perspective transformation matrix
    cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(srcPts, dstPts);

    // Apply perspective warp
    cv::Mat result;
    cv::warpPerspective(srcImg, result, perspectiveMatrix, outSize, cv::INTER_LINEAR,
                        cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

    return result;
}
