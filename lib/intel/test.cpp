#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <SimpleFace.h>

bool test(const char* filename) {
    struct stat st;
    char *buffer = NULL;
    size_t size = 0;
    if (stat(filename, &st) != 0) {
        std::cerr << "Could not read the input image" << std::endl;
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
        std::cerr << "Could not read the input image" << std::endl;
        return false;
    }
    FaceDetector fd(image);*/
    if (!fd.hasFace()) {
        std::cerr << "Could not find face in image" << std::endl;
        return false;
    }
    printf("%zd faces found\n", fd.numberOfFaces());
    Face face = fd.defaultFace();
    printf("%f\n", getFaceDistance(face));
    printf("%s\n", getFaceDirection(face)? "AWAY_SCREEN" : "TOWARD_SCREEN");
    return true;
}

#include <time.h>
int main (int argc, char *argv[]) {
    clock_t t0 = clock();
    FaceDetector::preloadModel("");
    clock_t t1 = clock();
    printf("loading time: %ld ms\n\n", (t1 - t0) / (CLOCKS_PER_SEC / 1000));

    t0 = t1;
    for (int i = 1; i < argc; ++i) {
        test(argv[i]);
        t1 = clock();
        printf("analysis time: %ld ms\n\n", (t1 - t0) / (CLOCKS_PER_SEC / 1000));
        t0 = t1;
    }
    return 0;
}
