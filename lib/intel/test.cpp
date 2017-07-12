#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#include <SimpleFace.h>

int main (int argc, char *argv[]) {
    cv::Mat image;
    if (argc > 1)
        image = cv::imread(argv[1], -1);
    if (image.empty()) {
        std::cerr << "Could not read the input image" << std::endl;
        return 1;
    }
    FaceDetector fd(image);
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
