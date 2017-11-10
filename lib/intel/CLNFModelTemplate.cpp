#include <unistd.h>

#include <string>

#include <LandmarkDetectorParameters.h>

#include "CLNFModelTemplate.h"

CLNFModelTemplate::CLNFModelTemplate(const std::string& location) :
    location_(location), loaded_(false) {
    if (location.empty()) {
        // default location
        LandmarkDetector::FaceModelParameters parameters;
        location_ = parameters.model_location;
    }
}

void CLNFModelTemplate::load() {
    if (loaded_) return;

    int fdout = dup(1);
    close(1);
    new (model_) LandmarkDetector::CLNF(location_);
    loaded_ = true;
    dup2(fdout, 1);
    close(fdout);
}
