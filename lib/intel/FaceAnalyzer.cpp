#include <future>

#include <Log.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <dlib/image_processing/frontal_face_detector.h>

#include <LandmarkDetectorUtils.h>
#include <LandmarkDetectorFunc.h>
#include <LandmarkDetectorParameters.h>
#include <GazeEstimation.h>

#include "FaceAnalyzer.h"

#define TIMEDIFF(t0, t1) ((t1.tv_nsec - t0.tv_nsec)/1000000 + (t1.tv_sec - t0.tv_sec) * 1000)

cv::Mat_<uchar> grayscalize(const cv::Mat& in);

FaceAnalyzer::FaceAnalyzer(const std::string& basedir) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_REALTIME, &t0);

    // start worker thread
    image_.release();
    running_ = false;
    auto main = std::bind(&FaceAnalyzer::WorkerThreadMain,
                          std::ref(*this), basedir);
    worker_ = std::thread(main);

    // wait for worker thread startup
    std::unique_lock<std::mutex> lock(mutex_);
    while (!running_)
        cv_.wait(lock);

    clock_gettime(CLOCK_REALTIME, &t1);
    LOG_I("thread start time: %ld ms", TIMEDIFF(t0, t1));
}
FaceAnalyzer::~FaceAnalyzer() {
    // notify and wait worker thread to stop
    running_ = false;
    cv_.notify_one();
    worker_.join();
}

void FaceAnalyzer::WaitForReady(std::function<void()> func) {
    mutex_.lock();
    while (!image_.empty()) {
        mutex_.unlock();
        usleep(1);
        mutex_.lock();
    }
    if (func)
        func();
    mutex_.unlock();
}

Face FaceAnalyzer::Analyze(const cv::Mat& image,
                           const cv::Rect_<double>& bounding_box,
                           const cv::Mat_<float>& depth_image) {
    std::promise<Face> promise;
    std::future<Face> result = promise.get_future();
    auto callback = [&](Face face) {
        promise.set_value(face);
    };

    mutex_.lock();
    callback_ = callback;
    bounding_box_ = bounding_box;
    image_ = image;
    depth_image_ = depth_image;
    mutex_.unlock();
    cv_.notify_one();

    result.wait();
    return result.get();
}

bool FaceAnalyzer::Analyze(const std::function<void(Face)>& callback,
                           const cv::Mat& image,
                           const cv::Rect_<double>& bounding_box,
                           const cv::Mat_<float>& depth_image) {
    assert(callback && !image.empty());
    if (!image_.empty()) {
        // worker thread has not scheduled since last call
        std::this_thread::yield();
        return false;
    }
    if (!mutex_.try_lock()) {
        // worker thread is busy
        return false;
    }

    callback_ = callback;
    bounding_box_ = bounding_box;
    image_ = image;
    depth_image_ = depth_image;
    mutex_.unlock();
    cv_.notify_one();
    return true;
}

void FaceAnalyzer::WorkerThreadMain(std::string basedir) {
    // notify main thread
    running_ = true;
    cv_.notify_one();

    std::unique_lock<std::mutex> lock(mutex_);
    LandmarkDetector::FaceModelParameters params;
    //params.curr_face_detector = LandmarkDetector::FaceModelParameters::HOG_SVM_DETECTOR;
    params.curr_face_detector = LandmarkDetector::FaceModelParameters::HAAR_DETECTOR;
    //params.reinit_video_every = -1;
#if TRACK_GAZE
    params.track_gaze = true;
#endif

    if (!basedir.empty()) {
        params.model_location = basedir + "/" + params.model_location;
        params.face_detector_location = basedir + "/" + params.face_detector_location;
    }
    LOG_I("model path: %s", params.model_location.c_str());
    assert(!access(params.model_location.c_str(), R_OK));
    LOG_I("classifier path: %s", params.face_detector_location.c_str());
    assert(!access(params.face_detector_location.c_str(), R_OK));

    // load model files
    int fdout = dup(1);
    close(1);
    struct timespec t0, t1;
    clock_gettime(CLOCK_REALTIME, &t0);
    auto model = LandmarkDetector::CLNF(params.model_location);
    clock_gettime(CLOCK_REALTIME, &t1);
    LOG_I("model loading time: %ld ms", TIMEDIFF(t0, t1));
    // preload HAAR cascade classifier
    model.face_detector_HAAR.load(params.face_detector_location);
    model.face_detector_location = params.face_detector_location;
    clock_gettime(CLOCK_REALTIME, &t0);
    LOG_I("classifier loading time: %ld ms", TIMEDIFF(t1, t0));
    dup2(fdout, 1);
    close(fdout);

    // run loop
    while (running_) {
        cv_.wait(lock);
        if (!image_.empty()) {
            clock_gettime(CLOCK_REALTIME, &t0);
            auto face = Analyze(model, params);
            clock_gettime(CLOCK_REALTIME, &t1);
            LOG_I("face analysis time: %ld ms", TIMEDIFF(t0, t1));
            // invoke callback
            face.image_ = std::move(image_);
            callback_(face);
        }
    }
}

// model, param
// image, bounting_box, depth
// optical_center, focal_length
Face FaceAnalyzer::Analyze(LandmarkDetector::CLNF& model, LandmarkDetector::FaceModelParameters& params) {
    auto grayscale = grayscalize(image_);
    if (!LandmarkDetector::DetectLandmarksInVideo(grayscale, depth_image_, bounding_box_, model, params))
        return Face();

    double fx = 500 * (image_.cols / 640.0);
    double fy = 500 * (image_.rows / 480.0);
    auto focal_length = cv::Point2d((fx + fy) / 2.0f, fx);
    auto optical_center = cv::Point2f(image_.cols / 2.0f, image_.rows / 2.0f);
    cv::Vec6d pose = LandmarkDetector::GetCorrectedPoseWorld(model, focal_length.x, focal_length.y, optical_center.x, optical_center.y);

#if TRACK_GAZE
    // Gaze tracking, absolute gaze direction
    cv::Point3f left_gaze(0, 0, -1);
    cv::Point3f right_gaze(0, 0, -1);
    FaceAnalysis::EstimateGaze(model, left_gaze, focal_length_.x, focal_length_.y, optical_center_.x, optical_center_.y, true);
    FaceAnalysis::EstimateGaze(model, right_gaze, focal_length_.x, focal_length_.y, optical_center_.x, optical_center_.y, false);
    return Face(bounding_box_, pose, left_gaze, right_gaze);
#else
    return Face(bounding_box_, pose);
#endif
}

cv::Mat_<uchar> grayscalize(const cv::Mat& in) {
    cv::Mat_<uchar> out;
    switch (in.channels()) {
        case 3:  // RGB
            // Make sure it's in a correct format
            if(in.depth() == CV_8U) {
                cv::cvtColor(in, out, CV_BGR2GRAY);
            } else if(in.depth() == CV_16U) {
                out = in / 256;
                out.convertTo(out, CV_8U);
                cv::cvtColor(out, out, CV_BGR2GRAY);
            }
            break;
        case 4:  // RGBA
            cv::cvtColor(in, out, CV_BGRA2GRAY);
            break;
        case 2:  // YUV
            cv::cvtColor(in, out, CV_YUV2GRAY_YUY2);
            break;
        case 1:  // Gray
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
