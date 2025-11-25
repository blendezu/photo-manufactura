#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QImage>
#include <opencv2/opencv.hpp>
#include <string>

class Document {
   public:
    static cv::Mat Q2Mat(const QImage& img);

    static QImage Mat2Q(const cv::Mat& mat);

    static cv::Mat loadImg(std::string path);

    static void saveImg(const cv::Mat& img, std::string fileName);
};

#endif