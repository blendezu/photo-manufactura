#include "document.h"
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <QImage>

QImage Document::Mat2Q(const cv::Mat& matImg) {
    // Sicherstellen, dass das Bild kontinuierlich im Speicher liegt
    cv::Mat matContiguous;
    if (!matImg.isContinuous()) {
        matContiguous = matImg.clone();
    } else {
        matContiguous = matImg;
    }

    switch (matContiguous.type()) {
        case CV_8UC1: {
            return QImage(matContiguous.data, matContiguous.cols, matContiguous.rows,
                          matContiguous.step, QImage::Format_Grayscale8);
        }

        case CV_8UC3: {
            cv::Mat rgb;
            cv::cvtColor(matContiguous, rgb, cv::COLOR_BGR2RGB);
            return QImage(rgb.data, rgb.cols, rgb.rows,
                          rgb.step, QImage::Format_RGB888);
        }

        case CV_8UC4: {
            return QImage(matContiguous.data, matContiguous.cols, matContiguous.rows,
                          matContiguous.step, QImage::Format_ARGB32);
        }

        case CV_16UC1: {
            cv::Mat mat8;
            matContiguous.convertTo(mat8, CV_8UC1, 1.0 / 256.0);
            return QImage(mat8.data, mat8.cols, mat8.rows,
                          mat8.step, QImage::Format_Grayscale8);
        }

        case CV_16UC3: {
            cv::Mat mat8;
            matContiguous.convertTo(mat8, CV_8UC3, 1.0 / 256.0);
            cv::Mat rgb;
            cv::cvtColor(mat8, rgb, cv::COLOR_BGR2RGB);
            return QImage(rgb.data, rgb.cols, rgb.rows,
                          rgb.step, QImage::Format_RGB888);
        }

        default:
            throw std::runtime_error("Unsupported cv::Mat format");
    }
}

cv::Mat Document::Q2Mat(const QImage& img) {
    if (img.isNull()) {
        throw std::runtime_error("QImage is null");
    }

    cv::Mat mat;

    switch (img.format()) {

        case QImage::Format_Grayscale8: {
            // 1-Kanal Graustufen
            mat = cv::Mat(img.height(), img.width(), CV_8UC1, const_cast<uchar*>(img.bits()), img.bytesPerLine());
            return mat.clone(); // clone, damit cv::Mat eigene Daten besitzt
        }

        case QImage::Format_RGB888: {
            // 3-Kanal RGB
            mat = cv::Mat(img.height(), img.width(), CV_8UC3, const_cast<uchar*>(img.bits()), img.bytesPerLine());
            cv::Mat bgr;
            cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR); // OpenCV nutzt BGR
            return bgr;
        }

        case QImage::Format_ARGB32: {
            // 4-Kanal BGRA
            mat = cv::Mat(img.height(), img.width(), CV_8UC4, const_cast<uchar*>(img.bits()), img.bytesPerLine());
            cv::Mat bgr;
            cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR); // optional in BGR
            return bgr;
        }

        default:
            throw std::runtime_error("Unsupported QImage format");
    }
}
