#include <fcntl.h>

#include <opencv2/imgcodecs.hpp>

#include <jsnipp.h>
#include <jsni/Log.h>

#include "videorendererinterface.h"
#include "FaceAnalyzer.h"
#include "SimpleFace.h"

using namespace jsnipp;

class JSPoint : public JSObject {
public:
    JSPoint(const cv::Point3f& point) : JSObject({
        { "x", JSNumber(point.x) },
        { "y", JSNumber(point.y) },
    }){}
};
class JSRect : public JSObject {
public:
    JSRect(const cv::Rect_<double>& rect) : JSObject({
        { "x", JSNumber(rect.x) },
        { "y", JSNumber(rect.y) },
        { "width",  JSNumber(rect.width) },
        { "height", JSNumber(rect.height) }
    }){}
};
class JSHeadPose : public JSObject {
public:
    JSHeadPose(const cv::Vec6d& pose) : JSObject({
        { "x", JSNumber(pose[0]) },
        { "y", JSNumber(pose[1]) },
        { "z", JSNumber(pose[2]) },
        { "alpha", JSNumber(pose[3]) },
        { "beta",  JSNumber(pose[4]) },
        { "gamma", JSNumber(pose[5]) }
    }){}
};

class JSFace : public JSObject {
public:
    JSFace(const Face& face) : JSObject({
        { "boundingBox", JSRect(face.boundingBox()) },
        { "headPose",    JSHeadPose(face.headPose()) },
#if TRACK_GAZE
        { "leftGaze",    JSPoint(std::get<0>(face.gazeDirection())) },
        { "rightGaze",   JSPoint(std::get<1>(face.gazeDirection())) }
#endif
    }){}
};

class JSFaceCallback : public JSCallback<void, Face> {
    using JSCallback::JSCallback;
    JSValue jsnify(Face& face) const override {
        return JSFace(face);
    }
};

class JSFaceDetector {
public:
    static std::string setup(JSObject cls);

private:
    friend class JSNativeConstructor<JSFaceDetector>;
    JSFaceDetector(JSObject, JSArray args);

    JSValue hasFace(JSObject) const {
        if (!simple_face)  return JSNull();
        return JSBoolean(simple_face->hasFace());
    }
    JSValue numberOfFaces(JSObject) const {
        if (!simple_face)  return JSNull();
        return JSNumber(simple_face->numberOfFaces());
    }
    JSValue getFaceByIndex(JSObject, JSArray args) const {
        if (simple_face && args.length() == 1) {
            size_t index = JSNumber(args[0]);
            if (index < simple_face->numberOfFaces())
                return JSFace(simple_face->getFaceByIndex(index));
        }
        return JSNull();
    }
    JSValue defaultFace(JSObject) const {
        if (!simple_face)  return JSNull();
        return JSFace(simple_face->defaultFace());
    }

    std::unique_ptr<FaceDetector> simple_face;
};

JSFaceDetector::JSFaceDetector(JSObject, JSArray args) {
    if (args.length() == 0) return;
    JSValue jsval = args[0];
    if (!jsval.is_typedarray()) return;
    auto img = JSTypedArray<uint8_t>(jsval);

#ifdef DUMP_JPEG
    int fd = creat("/tmp/egroshot.jpg", 00644);
    write(fd, img.buffer(), img.length());
    fsync(fd);
    close(fd);
#endif

    simple_face.reset(new FaceDetector(img.buffer(), img.length()));
}

std::string JSFaceDetector::setup(JSObject cls) {
    cls.defineProperty("hasFace", JSPropertyAccessor(JSNativeGetter<const JSFaceDetector, &JSFaceDetector::hasFace>()));
    cls.defineProperty("numberOfFaces", JSPropertyAccessor(JSNativeGetter<const JSFaceDetector, &JSFaceDetector::numberOfFaces>()));
    cls.defineProperty("defaultFace", JSPropertyAccessor(JSNativeGetter<const JSFaceDetector, &JSFaceDetector::defaultFace>()));
    cls.setProperty("getFaceByIndex", JSNativeMethod<const JSFaceDetector, &JSFaceDetector::getFaceByIndex>());
    return "";
}

class JSFaceAnalyzer : public woogeen::base::VideoRendererARGBInterface {
public:
    static std::string setup(JSObject cls);

    void RenderFrame(std::unique_ptr<woogeen::base::ARGBBuffer> buffer) override;

private:
    friend class JSNativeConstructor<JSFaceAnalyzer>;
    JSFaceAnalyzer(JSObject, JSArray args);

    JSValue Analyze(JSObject, JSArray args);

    std::unique_ptr<FaceAnalyzer> analyzer_;
    JSGlobalValue callback_;
};

static std::string getModulePath();
JSFaceAnalyzer::JSFaceAnalyzer(JSObject, JSArray args) {
    if (args.length() == 0) return;
    JSValue jsval = args[0];
    if (!jsval.is_function()) return;
    callback_ = JSGlobalValue(jsval);
    std::string location = getModulePath() + "/model/main_clnf_general.txt";
    analyzer_.reset(new FaceAnalyzer(location));
}

void JSFaceAnalyzer::RenderFrame(std::unique_ptr<woogeen::base::ARGBBuffer> buffer) {
    auto image = cv::Mat(buffer->resolution.height,
                         buffer->resolution.width,
                         CV_8UC4, buffer->buffer);
    buffer->buffer = nullptr;

    auto callback = [this](Face face) {
        auto cb = new JSFaceCallback(this->callback_);
        delete face.image().data;
        (*cb)(face);
    };
    analyzer_->Analyze(callback, image);
}

JSValue JSFaceAnalyzer::Analyze(JSObject, JSArray args) {
    if (args.length() == 0 || !args[0].is_typedarray())
        return false_js;

    auto array = JSTypedArray<uint8_t>(args[0]);
    cv::Mat image = cv::Mat(1, array.length(), CV_8UC1, array.buffer());
    image = cv::imdecode(image, cv::IMREAD_GRAYSCALE);

    if (args.length() == 1 || !args[1].is_function()) {
        return JSFace(analyzer_->Analyze(image));
    }

    JSGlobalValue callback(args[1]);
    auto cb = [callback](Face face) {
        auto cb = new JSFaceCallback(callback);
        (*cb)(face);
    };
    return JSBoolean(analyzer_->Analyze(cb, image));
}

std::string JSFaceAnalyzer::setup(JSObject cls) {
    cls.setProperty("analyze", JSNativeMethod<JSFaceAnalyzer, &JSFaceAnalyzer::Analyze>());
    return "";
}

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
static std::string getModulePath() {
    Dl_info info;
    dladdr(reinterpret_cast<void*>(getModulePath), &info);
    const char *p = strstr(info.dli_fname, "/lib/native/");
    assert(p);
    return std::string(info.dli_fname, p - info.dli_fname);
}

__attribute__ ((visibility("default")))
int JSNI_Init(JSNIEnv* env, JsValue exports) {
    LOG_I("SimpleFace JSNI module is loaded");
    //FaceDetector::preloadModel(getModulePath() + "/model/main_clnf_general.txt");

    JSValue::setup(env);
    JSObject jsobj(exports);

    jsobj.setProperty("FaceDetector", JSNativeConstructor<JSFaceDetector>(&JSFaceDetector::setup));
    jsobj.setProperty("FaceAnalyzer", JSNativeConstructor<JSFaceAnalyzer>(&JSFaceAnalyzer::setup));

    return JSNI_VERSION_1_1;
}
