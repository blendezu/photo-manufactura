#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <opencv2/opencv.hpp>
#include <QImage>


class Document {
public:
    cv::Mat Q2Mat(const QImage& matImg);
    QImage Mat2Q(const cv::Mat& qImg);

};

#endif