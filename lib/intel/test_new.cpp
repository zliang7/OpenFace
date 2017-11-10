#include <sys/stat.h>
#include <unistd.h>

#include <opencv2/highgui/highgui.hpp>
#include <FaceAnalyzer.h>

bool test(FaceAnalyzer& detector, const char* filename) {
    cv::Mat image;
    image = cv::imread(filename, -1);
    if (image.empty()) {
        fprintf(stderr, "Could not read the input image: %s\n", filename);
        return false;
    }

    auto callback = [filename](Face face) {
        if (face) {
            fprintf(stderr, "%f\n", getFaceDistance(face));
            fprintf(stderr, "%s\n", getFaceDirection(face)? "AWAY_SCREEN" : "TOWARD_SCREEN");
        } else {
            fprintf(stderr, "Could not find face in image: %s\n", filename);
        }
    };

#if 0 // sync mode
    auto face = detector.Analyze(image);
    callback(face);
    return face;
#else // async mode
    bool res = detector.Analyze(callback, image);
    detector.WaitForReady();
    return res;
#endif
}

#include <time.h>
#define timediff(t0, t1) ((t1.tv_nsec - t0.tv_nsec)/1000000 + (t1.tv_sec - t0.tv_sec) * 1000)
int main (int argc, char *argv[]) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_REALTIME, &t0);
    FaceAnalyzer detector;
    detector.WaitForReady();
    clock_gettime(CLOCK_REALTIME, &t1);
    fprintf(stderr, "loading time: %ld ms\n\n", timediff(t0, t1));

    t0 = t1;
    for (int i = 1; i < argc; ++i) {
        test(detector, argv[i]);
        clock_gettime(CLOCK_REALTIME, &t1);
        fprintf(stderr, "analysis time: %ld ms\n\n", timediff(t0, t1));
        t0 = t1;
    }
    return 0;
}
