#pragma once

#include <string>

#include <LandmarkDetectorModel.h>

class CLNFModelTemplate {
public:
    CLNFModelTemplate(const std::string& location = std::string());
    ~CLNFModelTemplate() {
        unload();
    }

    void load();
    void load(const std::string& location) {
        if (location != location_) {
            // change model
            unload();
            location_ = location;
        }
        load();
    }

    const LandmarkDetector::CLNF& get() {
        return *ptr();
    }

private:
    LandmarkDetector::CLNF* ptr() {
        if (!loaded_) load();
        return reinterpret_cast<LandmarkDetector::CLNF*>(model_);
    }
    void unload() {
        if (loaded_) ptr()->~CLNF();
    }

    bool loaded_;
    std::string location_;
    uint8_t model_[sizeof(LandmarkDetector::CLNF)];
};
