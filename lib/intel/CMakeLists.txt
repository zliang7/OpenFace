SET(SOURCE
    SimpleFace.cpp
    JSBinding.cpp
)

SET(HEADERS
    SimpleFace.h
)

include_directories(./include)
include_directories(../jsnipp)
include_directories(../local/FaceAnalyser/include)
include_directories(../local/LandmarkDetector/include)
include_directories(${TBB_ROOT_DIR}/include)

add_library(SimpleFace SHARED ${SOURCE} ${HEADERS})
set_property(TARGET SimpleFace PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET SimpleFace PROPERTY VISIBILITY_INLINES_HIDDEN TRUE)
target_link_libraries(SimpleFace jsnipp)
target_link_libraries(SimpleFace LandmarkDetector)
target_link_libraries(SimpleFace FaceAnalyser)
target_link_libraries(SimpleFace ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})
target_link_libraries(SimpleFace dlib)

install (TARGETS SimpleFace DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include/OpenFace)


include_directories(.)
add_executable(ApiTest test.cpp)
target_link_libraries(ApiTest SimpleFace)
install (TARGETS ApiTest DESTINATION bin)
