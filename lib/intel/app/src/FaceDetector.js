"use strict";

// FaceDetector mock class

class Face {
    constructor() {
        this.headPose = [0, 2, 4, 6, 8, 10];
    }
}

class FaceDetector {

    constructor() {
        this.hasFace = true;
    }

    defaultFace() {
        return new Face();
    }

}

module.exports = FaceDetector;
