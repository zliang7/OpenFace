#pragma once

#include <tuple>
#include <vector>

#include <opencv2/core/core.hpp>

class Face {
public:
    cv::Rect_<double> boundingBox() const {
        return box_;
    }
    cv::Vec6d headPose() const {
        return pose_;
    }
    std::tuple<cv::Point3f, cv::Point3f> gazeDirection() const {
        return std::make_tuple(left_gaze_, left_gaze_);
    }

private:
    Face(cv::Rect_<double> box, cv::Vec6d pose, cv::Point3f left, cv::Point3f right):
        box_(box), pose_(pose), left_gaze_(left), right_gaze_(right) {}
    friend class FaceDetector;

    //double confidence_;
    cv::Rect_<double> box_;
    cv::Vec6d pose_;
    cv::Point3f left_gaze_;
    cv::Point3f right_gaze_;
};

class FaceDetector {
public:
    FaceDetector(const cv::Mat& image):
        FaceDetector(image, cv::Mat_<float>()) {}
    FaceDetector(void* buffer, size_t size);
    FaceDetector(const cv::Mat& image, const cv::Mat_<float>& depth);

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

private:
    static cv::Mat_<uchar> grayscalize(const cv::Mat& in);

    cv::Mat_<uchar> image_;
    cv::Mat_<float> depth_;
    cv::Point2f optical_center_;
    cv::Point2f focal_length_;
    std::vector<cv::Rect_<double>> faces_;
    mutable size_t default_face_;
    //void* model_;
};


// High level API, defined by Nathaniel

enum FaceDirection {
    TOWARD_SCREEN,
    AWAY_SCREEN
};

inline float getFaceDistance(const Face& face) {
    return face.headPose()[2];
}

inline FaceDirection getFaceDirection(const Face& face) {
    const double threshold = M_PI / 12;
    auto pose = face.headPose();
    return pose[3] > threshold || pose[3] < -threshold ||
           pose[4] > threshold || pose[4] < -threshold ||
           pose[5] > threshold || pose[5] < -threshold ?
           AWAY_SCREEN : TOWARD_SCREEN;
}

// this is unsupported because the body pose is unknown
//float getPosture(const Face& face);
