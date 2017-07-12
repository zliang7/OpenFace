SET(SOURCE
    SimpleFace.cpp
)

SET(HEADERS
    SimpleFace.h
)

include_directories(./include)
include_directories(../local/FaceAnalyser/include)
include_directories(../local/LandmarkDetector/include)
include_directories(${TBB_ROOT_DIR}/include)

add_library(SimpleFace ${SOURCE} ${HEADERS})

install (TARGETS SimpleFace DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include/OpenFace)



include_directories(.)
add_executable(ApiTest test.cpp)
target_link_libraries(ApiTest SimpleFace)
target_link_libraries(ApiTest LandmarkDetector)
target_link_libraries(ApiTest FaceAnalyser)
target_link_libraries(ApiTest dlib)
target_link_libraries(ApiTest ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})
install (TARGETS ApiTest DESTINATION bin)
