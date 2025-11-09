#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QImage>
#include <opencv2/opencv.hpp>

class Document {
   public:
    cv::Mat Q2Mat(const QImage& img);
    QImage Mat2Q(const cv::Mat& mat);
};

#endif