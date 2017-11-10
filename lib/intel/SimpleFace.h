#pragma once

#include <tuple>
#include <vector>

#include <opencv2/core/core.hpp>

#include <Face.h>

class __attribute__ ((visibility ("default"))) FaceDetector final {
public:
    FaceDetector(const cv::Mat& image):
        FaceDetector(image, cv::Mat_<float>()) {}
    FaceDetector(void* buffer, size_t size);
    FaceDetector(const cv::Mat& image, const cv::Mat_<float>& depth);
    ~FaceDetector();

    bool hasFace() const {
        return faces_.size() > 0;
    }
    size_t numberOfFaces() const {
        return faces_.size();
    }
    Face getFaceByIndex(size_t index) const;
    Face defaultFace() const;
    Face operator [](size_t index) const {
        return getFaceByIndex(index);
    }

    static void preloadModel(const std::string& location);

private:
    static cv::Mat_<uchar> grayscalize(const cv::Mat& in);

    cv::Mat_<uchar> image_;
    cv::Mat_<float> depth_;
    cv::Point2f optical_center_;
    cv::Point2f focal_length_;
    std::vector<cv::Rect_<double>> faces_;
    mutable size_t default_face_;
    mutable void* model_;
};
