cmake_minimum_required(VERSION 3.10.0)
project(FortniteAnalyser VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED)
find_package(SFML REQUIRED Graphics Window System)

add_subdirectory(src)

add_executable(FortniteAnalyser src/main.cpp)
target_compile_features(FortniteAnalyser PRIVATE cxx_std_20)
target_link_libraries(FortniteAnalyser PRIVATE helpers ${OpenCV_LIBS} SFML::Graphics SFML::Window SFML::System)

include(CTest)
enable_testing()