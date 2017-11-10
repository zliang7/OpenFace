add_library(jsnihelper SHARED jsnihelper_dummy.c)

SET(SOURCE
    CLNFModelTemplate.cpp
    FaceAnalyzer.cpp
    SimpleFace.cpp
    JSBinding.cpp
)

SET(HEADERS
    FaceAnalyzer.h
    SimpleFace.h
)

include_directories(./include)
include_directories(../jsnipp)
include_directories(../local/FaceAnalyser/include)
include_directories(../local/LandmarkDetector/include)
include_directories(${TBB_ROOT_DIR}/include)

add_definitions(-DYUNOS_EXPORT= -DLOG_TAG="ERGO")
add_library(SimpleFace SHARED ${SOURCE} ${HEADERS})
set_property(TARGET SimpleFace PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET SimpleFace PROPERTY VISIBILITY_INLINES_HIDDEN TRUE)
set_property(TARGET SimpleFace PROPERTY LINK_FLAGS "-u yunosLogPrint")
target_link_libraries(SimpleFace jsnipp)
target_link_libraries(SimpleFace LandmarkDetector)
target_link_libraries(SimpleFace FaceAnalyser)
target_link_libraries(SimpleFace ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})
target_link_libraries(SimpleFace dlib)
target_link_libraries(SimpleFace jsnihelper)

install (TARGETS SimpleFace DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include/OpenFace)


include_directories(.)
add_executable(ApiTest test_new.cpp)
set_property(TARGET ApiTest PROPERTY LINK_FLAGS "-u yunosLogPrint -u AsyncThreadWork")
target_link_libraries(ApiTest SimpleFace)
install (TARGETS ApiTest DESTINATION bin)
