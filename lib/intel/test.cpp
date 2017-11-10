#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <SimpleFace.h>

bool test(const char* filename) {
    struct stat st;
    char *buffer = NULL;
    size_t size = 0;
    if (stat(filename, &st) != 0) {
        fprintf(stderr, "Could not read the input image: %s\n", filename);
        return false;
    }
    size = st.st_size;
    buffer = (char*)malloc(size);
    std::ifstream(filename, std::ios::binary).read(buffer, size);
    FaceDetector fd(buffer, size);
    free(buffer);

/*  cv::Mat image;
    image = cv::imread(filename, -1);
    if (image.empty()) {
        fprintf(stderr, "Could not read the input image: %s\n", filename);
        return false;
    }
    FaceDetector fd(image);*/
    if (!fd.hasFace()) {
        fprintf(stderr, "Could not find face in image: %s\n", filename);
        return false;
    }
    fprintf(stderr, "%zd faces found\n", fd.numberOfFaces());
    Face face = fd.defaultFace();
    fprintf(stderr, "%f\n", getFaceDistance(face));
    fprintf(stderr, "%s\n", getFaceDirection(face)? "AWAY_SCREEN" : "TOWARD_SCREEN");
    return true;
}

#include <time.h>
#define timediff(t0, t1) ((t1.tv_nsec - t0.tv_nsec)/1000000 + (t1.tv_sec - t0.tv_sec) * 1000)
int main (int argc, char *argv[]) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_REALTIME, &t0);
    FaceDetector::preloadModel("");
    clock_gettime(CLOCK_REALTIME, &t1);
    fprintf(stderr, "loading time: %ld ms\n\n", timediff(t0, t1));

    t0 = t1;
    for (int i = 1; i < argc; ++i) {
        test(argv[i]);
        clock_gettime(CLOCK_REALTIME, &t1);
        fprintf(stderr, "analysis time: %ld ms\n\n", timediff(t0, t1));
        t0 = t1;
    }
    return 0;
}
