'use strict';

const bindings = nativeLoad("libSimpleFace.so");

const Camera = require("yunos/device/VideoCapture");
const SurfaceView = require("yunos/ui/view/SurfaceView");
const fs = require("fs");
const EventEmitter = require("yunos/core/EventEmitter");

const TAG = "ERGO";

const FaceDirection = {
    AWAY_SCREEN: 0,
    TOWARD_SCREEN: 1
}

const PostureQuality = {
    GOOD: 2,
    OKAY: 1,
    BAD: 0
}

class Ergo extends EventEmitter {

    constructor() {
        super();
        this.analyzer = new bindings.FaceAnalyzer(this.__callback.bind(this));
        this.FaceDirection = FaceDirection;
        this.PostureQuality = PostureQuality;
        // Threshold configuration
        this.config = {
            AWRY: 0.05,
            DIRECTION: 0.25,
            DISTANCE: 300,
            CENTER: 100,
            ANGLE_WEIGHT: 0.300,
            DIRECTION_WEIGHT: 0.300,
            DISTANCE_WEIGHT: 0.200,
            CENTER_WEIGHT: 0.200,
            ERGO_THRESHOLD: 0.750
        };
        this.captureReady = true;
        // Pre-populate array so alert doesn't trigger right away
        this.frames = Array(5).fill(-1);
    }

    init(window) {
        this.window = window;
        this.posture = this.PostureQuality.BAD;
        this.run = false;
        this.__initCamera();
    }

    start() {
        this.run = true;
    }

    stop() {
        this.run = false;
    }

    // Ergonomic Policy API
    isPresent() {
        let self = this;
        return self.present;
    }

    getPosture() {
        let self = this;
        return self.posture;
    }


// -----------------------

    __initCamera() {
        let self = this;
        //console.log(TAG, "initCamera");

        this.camera = Camera.create(0);
        this.camera.setStreamSize(Camera.StreamType.VIDEO_CAPTURE_STREAM_PREVIEW, 320, 240);

        this.surface = new SurfaceView();
        this.window.addChild(this.surface);
        this.surface.width = 320;
        this.surface.height = 240;
        this.surface.top = this.window.height - this.surface.height;
        this.surface.left = 0;
        this.surface.surfaceType = SurfaceView.SurfaceType.Nested;
        this.surface.on('ready', function() {
            //console.log(TAG, "surface ready");
            //console.log(TAG, self.surface.getClientToken().toString());
            self.camera.startStream(Camera.StreamType.VIDEO_CAPTURE_STREAM_PREVIEW, 0, self.surface.getClientToken().toString());
            self.__capture();
        });

    }

    __capture() {
        let self = this;
        if (self.run === true) {
            setTimeout(function() {
                self.__takePhoto();
                self.__capture();
            }, 5000);
        }
    }

    __takePhoto() {
        let self = this;
        if (!this.camera) {
            console.log(TAG, "Error: no Camera object!");
            return;
        }
        //console.log(TAG, "taking photo...");
        this.camera.on('capturedata', function(data) {
            // NOT run every frame, because frames
            //   can be captured much more quickly
            if (self.captureReady) {
                self.captureReady = false;
                self.__analyze(data);
            }
        });
        this.camera.startStream(Camera.StreamType.VIDEO_CAPTURE_STREAM_CAPTURE, 0, "");
    }

    __callback(face) {
        let self = this;
        if (face.headPose.z == 0) {
            console.log(TAG, "No Face!!");
            return;
        }
        console.log(TAG, face.headPose);
        let faceInfo = {
            xPosition: face.headPose.x,
            yPosition: face.headPose.y,
            zPosition: face.headPose.z,
            xRotation: face.headPose.alpha,
            yRotation: face.headPose.beta,
            zRotation: face.headPose.gamma
        }
        console.log(TAG, faceInfo);
        let result = {
            faceDetected: self.__getFaceDistance(faceInfo) > 0,
            faceDistance: self.__getFaceDistance(faceInfo),
            faceDirection: self.__getFaceDirection(faceInfo),
            faceAwry: self.__isFaceAwry(faceInfo),
            faceCentered: self.__isFaceCentered(faceInfo),
            faceInfo: faceInfo
        };
        self.__score(result);
        self.emit('frame', result);
        self.captureReady = true;
    }

    __analyze(data) {
        //console.log(TAG, "analyzing..");
        let self = this;
        let array = new Uint8Array(data);
        //let buffer = new Buffer(array);
        //fs.writeFile("/tmp/ergo.jpg", buffer, {encoding: "utf-8", mode: "777"})
        this.analyzer.analyze(array, this.__callback.bind(this));
    }

    __score(data) {
        let self = this;
        let score = 1;
        //console.log(data);
        if (!data.faceDetected) {
            console.log(TAG, "No face detected");
            score = -1;
        } else {
            console.log(TAG, "Face detected");
            // If head rotation is outside of detectable range,
            //   deduct all points
            let awry_mag = Math.abs(data.faceInfo.zRotation);
            // 0.130 is ~ detectable range
            if (awry_mag > 0.130) {
                score -= self.config.ANGLE_WEIGHT * (awry_mag/0.130);
            }
            if (data.faceDirection === self.FaceDirection.AWAY_SCREEN) {
                score -= self.config.DIRECTION_WEIGHT;
            }
            if (data.faceDistance < self.config.DISTANCE) {
                score -= self.config.DISTANCE_WEIGHT;
            }
            if (!data.faceCentered) {
                score -= self.config.CENTER_WEIGHT;
            }
        }
        console.log(TAG, "score: " + score);
        self.frames.shift();
        self.frames.push(score);
        let noFaceCount = 0;
        let total = 0;
        for (let i=0; i<self.frames.length; i++) {
            if (self.frames[i] === -1) {
                noFaceCount++;
            } else {
            total += self.frames[i];
            }
        }
        //console.log(TAG, "total: " + total);
        //console.log(TAG, "self.frames.length: " + self.frames.length);
        //console.log(TAG, "noFaceCount: " + noFaceCount);

        let average = total / (self.frames.length - noFaceCount);
        console.log(TAG, "Average: " + average);
        if (noFaceCount === self.frames.length) {
            self.present = false;
        } else if (average < self.config.ERGO_THRESHOLD) {
            self.posture = self.PostureQuality.BAD;
            self.present = true;
        } else {
            self.posture = self.PostureQuality.GOOD;
            self.present = true;
        }
    }

    // Face Detection Policy API
    __isFaceDetected(faceInfo) {
        return faceInfo.zPosition > 0;
    }

    __getFaceDistance(faceInfo) {
        return faceInfo.zPosition;
    }

    __getFaceDirection(faceInfo) {
        return (faceInfo.xRotation > this.config.DIRECTION
             || faceInfo.xRotation < -this.config.DIRECTION
             || faceInfo.yRotation > this.config.DIRECTION
             || faceInfo.yRotation < -this.config.DIRECTION)
        ? this.FaceDirection.AWAY_SCREEN : this.FaceDirection.TOWARD_SCREEN;
    }

    __isFaceAwry(faceInfo) {
        return faceInfo.zRotation > this.config.AWRY
            || faceInfo.zRotation < -this.config.AWRY;
    }

    __isFaceCentered(faceInfo) {
        return faceInfo.xPosition < this.config.CENTER
            && faceInfo.xPosition > -this.config.CENTER;
    }

}

module.exports = Ergo;
