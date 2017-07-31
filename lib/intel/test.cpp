#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <SimpleFace.h>

int main (int argc, char *argv[]) {
    struct stat st;
    char *buffer = NULL;
    size_t size = 0;
    if (argc > 1 && stat(argv[1], &st) == 0) {
        size = st.st_size;
        buffer = (char*)malloc(size);
    }
    if (buffer == NULL) {
        std::cerr << "Could not read the input image" << std::endl;
        return 1;
    }
    std::ifstream(argv[1], std::ios::binary).read(buffer, size);
    FaceDetector fd(buffer, size);
    free(buffer);

/*  cv::Mat image;
    if (argc > 1)
        image = cv::imread(argv[1], -1);
    if (image.empty()) {
        std::cerr << "Could not read the input image" << std::endl;
        return 1;
    }
    FaceDetector fd(image);*/
    if (!fd.hasFace()) {
        std::cerr << "Could not find face in image" << std::endl;
        return 1;
    }
    printf("%zd faces found\n", fd.numberOfFaces());
    Face face = fd.defaultFace();
    printf("%f\n", getFaceDistance(face));
    printf("%s\n", getFaceDirection(face)? "AWAY_SCREEN" : "TOWARD_SCREEN");
    return 0;
}
