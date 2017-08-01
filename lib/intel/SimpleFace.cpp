#include <algorithm>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <dlib/image_processing/frontal_face_detector.h>

#include <LandmarkDetectorUtils.h>
#include <LandmarkDetectorFunc.h>
#include <GazeEstimation.h>

#include "SimpleFace.h"

LandmarkDetector::CLNF& getCLNFModelTemplate() {
    static uint8_t model_template[sizeof(LandmarkDetector::CLNF)];
    static bool is_loaded = false;
    if (is_loaded != true) {
        is_loaded = true;
        new (model_template) LandmarkDetector::CLNF();
    }
    return *reinterpret_cast<LandmarkDetector::CLNF*>(model_template);
}

FaceDetector::FaceDetector(const cv::Mat& image, const cv::Mat_<float>& depth):
    image_(grayscalize(image)), depth_(depth), default_face_(SIZE_MAX),
    optical_center_(image_.cols / 2.0f, image_.rows / 2.0f), model_(nullptr) {
    double fx = 500 * (image_.cols / 640.0);
    double fy = 500 * (image_.rows / 480.0);
    focal_length_ = cv::Point2d((fx + fy) / 2.0f, fx);

    //LandmarkDetector::DetectFaces(faces, grayscale_image, classifier);
    //HOG_SVM_DETECTOR
    std::vector<double> confidences;
    dlib::frontal_face_detector face_detector_hog = dlib::get_frontal_face_detector();
    LandmarkDetector::DetectFacesHOG(faces_, image_, face_detector_hog, confidences);
}

FaceDetector::FaceDetector(void* buffer, size_t size):
    FaceDetector(
        cv::imdecode(cv::Mat(1, size, CV_8UC1, buffer), cv::IMREAD_GRAYSCALE),
        cv::Mat_<float>()) {
}

FaceDetector::~FaceDetector() {
    delete reinterpret_cast<LandmarkDetector::CLNF*>(model_);
}

Face FaceDetector::getFaceByIndex(size_t index) const {
    assert(index < numberOfFaces());

    // Estimate head pose and eye gaze
    if (model_ == nullptr) {
        model_ = new LandmarkDetector::CLNF(getCLNFModelTemplate());
    }
    LandmarkDetector::CLNF& clnf_model = *reinterpret_cast<LandmarkDetector::CLNF*>(model_);
    LandmarkDetector::FaceModelParameters det_parameters;
    det_parameters.validate_detections = false;
    det_parameters.track_gaze = true;
    bool success = LandmarkDetector::DetectLandmarksInImage(image_, depth_, faces_[index], clnf_model, det_parameters);
    cv::Vec6d pose = LandmarkDetector::GetCorrectedPoseWorld(clnf_model, focal_length_.x, focal_length_.y, optical_center_.x, optical_center_.y);
    // Gaze tracking, absolute gaze direction
    cv::Point3f left_gaze(0, 0, -1);
    cv::Point3f right_gaze(0, 0, -1);
    if (success) {
        FaceAnalysis::EstimateGaze(clnf_model, left_gaze, focal_length_.x, focal_length_.y, optical_center_.x, optical_center_.y, true);
        FaceAnalysis::EstimateGaze(clnf_model, right_gaze, focal_length_.x, focal_length_.y, optical_center_.x, optical_center_.y, false);
    }
    return Face(faces_[index], pose, left_gaze, right_gaze);
}

Face FaceDetector::defaultFace() const {
    if (default_face_ > numberOfFaces()) {
        assert(hasFace());
        auto compare = [](const cv::Rect_<double>& a, const cv::Rect_<double>& b) {
            return a.area() < b.area();
        };
        auto it = std::max_element(faces_.begin(), faces_.end(), compare);
        default_face_ = it - faces_.begin();
    }
    return getFaceByIndex(default_face_);
}

cv::Mat_<uchar> FaceDetector::grayscalize(const cv::Mat& in) {
    cv::Mat_<uchar> out;
    switch (in.channels()) {
        case 3:
            // Make sure it's in a correct format
            if(in.depth() == CV_8U) {
                cv::cvtColor(in, out, CV_BGR2GRAY);
            } else if(in.depth() == CV_16U) {
                out = in / 256;
                out.convertTo(out, CV_8U);
                cv::cvtColor(out, out, CV_BGR2GRAY);
            }
            break;
        case 4:
            cv::cvtColor(in, out, CV_BGRA2GRAY);
            break;
        case 1:
            if(in.depth() == CV_16U)
                out = in / 256;
            else if(in.depth() == CV_8U)
                out = in.clone();
            else
                in.convertTo(out, CV_8U);
            break;
        default:
            assert(false);
    }
    return out;
}
