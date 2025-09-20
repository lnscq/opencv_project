cmake_minimum_required(VERSION 3.10.0)
project(opencv_project VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenCV REQUIRED)

aux_source_directory(./src SRC)

add_executable(opencv_project ${SRC})
target_link_libraries(opencv_project ${OpenCV_LIBS})