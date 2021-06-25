# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_library(
        v8android
        SHARED
        IMPORTED
)

set_target_properties(
        v8android

        PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libv8android.so
)

include_directories(
        src/main/cpp/include
        src/main/cpp
)

add_library(
        v8runtime

        SHARED

        src/main/cpp/OnLoad.cpp
)

find_library(
        log-lib

        log)

target_link_libraries(
        v8runtime
        v8android
        ${log-lib})

