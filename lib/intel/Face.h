#pragma once

#include <tuple>

#include <opencv2/core/core.hpp>

class Face {
public:
    Face() {}
    Face(const Face& face) {
        image_ = face.image_;
        box_ = face.box_;
        pose_ = face.pose_;
#if TRACK_GAZE
        left_gaze_ = face.left_gaze_;
        right_gaze_ = face.right_gaze_;
#endif
    }

    operator bool() const {
        return pose_[2] > 0;
    }

    cv::Mat image() const {
        return image_;
    }
    cv::Rect_<double> boundingBox() const {
        return box_;
    }
    cv::Vec6d headPose() const {
        return pose_;
    }
#if TRACK_GAZE
    std::tuple<cv::Point3f, cv::Point3f> gazeDirection() const {
        return std::make_tuple(left_gaze_, left_gaze_);
    }
#endif

private:
    Face(const cv::Rect_<double>& box, const cv::Vec6d& pose):
        box_(box), pose_(pose) {}
#if TRACK_GAZE
    Face(const cv::Rect_<double>& box, const cv::Vec6d& pose,
         const cv::Point3f& left, const cv::Point3f& right):
        box_(box), pose_(pose), left_gaze_(left), right_gaze_(right) {}
#endif
    friend class FaceAnalyzer;
    friend class FaceDetector;

    cv::Mat image_;
    //double confidence_;
    cv::Rect_<double> box_;
    cv::Vec6d pose_;
#if TRACK_GAZE
    cv::Point3f left_gaze_;
    cv::Point3f right_gaze_;
#endif
};


// High level API, defined by Nathaniel

enum FaceDirection {
    TOWARD_SCREEN,
    AWAY_SCREEN
};

constexpr double threshold = M_PI / 12;

inline float getFaceDistance(const Face& face) {
    return face.headPose()[2];
}

inline FaceDirection getFaceDirection(const Face& face) {
    auto pose = face.headPose();
    return pose[3] > threshold || pose[3] < -threshold ||
           pose[4] > threshold || pose[4] < -threshold?
           AWAY_SCREEN : TOWARD_SCREEN;
}

inline bool isAwry(const Face& face) {
    auto pose = face.headPose();
    return pose[5] > threshold || pose[5] < -threshold;
}

inline bool isCenter(const Face& face) {
    auto pose = face.headPose();
    return pose[0] > - 100 && pose[0] < 100;  // TODO: a better algorithm
}

// this is unsupported because the body pose is unknown
//float getPosture(const Face& face);
