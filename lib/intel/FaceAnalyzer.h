#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <tuple>

#include <opencv2/core/core.hpp>

#include <Face.h>

namespace LandmarkDetector {
class CLNF;
class FaceModelParameters;
}

class __attribute__ ((visibility ("default"))) FaceAnalyzer {
public:
    FaceAnalyzer(const std::string& location = std::string());
    ~FaceAnalyzer();

    void WaitForReady(std::function<void()> func = nullptr);

    Face Analyze(const cv::Mat& image,
                 const cv::Rect_<double>& bounding_box = cv::Rect_<double>(),
                 const cv::Mat_<float>& depth_image = cv::Mat_<float>());
    bool Analyze(const std::function<void(Face)>& callback,
                 const cv::Mat& image,
                 const cv::Rect_<double>& bounding_box = cv::Rect_<double>(),
                 const cv::Mat_<float>& depth_image = cv::Mat_<float>());

private:
    void WorkerThreadMain(std::string location);
    Face Analyze(LandmarkDetector::CLNF& model, LandmarkDetector::FaceModelParameters& params);

    bool running_;
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;

    cv::Mat image_;
    cv::Mat_<float> depth_image_;
    cv::Rect_<double> bounding_box_;
    std::function<void(Face)> callback_;
};
