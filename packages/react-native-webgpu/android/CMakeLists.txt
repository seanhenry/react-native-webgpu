cmake_minimum_required(VERSION 3.4.1)
project(react-native-webgpu)

set(CMAKE_CXX_STANDARD 17)

find_package(fbjni REQUIRED CONFIG)
find_package(ReactAndroid REQUIRED CONFIG)

file(GLOB SHARED_SOURCE "${CMAKE_SOURCE_DIR}/../cxx/*.cpp")
file(GLOB ANDROID_SOURCE "${CMAKE_SOURCE_DIR}/../cxx/android/*.cpp")

include_directories(../cxx
        "${CMAKE_SOURCE_DIR}/../cxx/android"
        "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni/react/turbomodule"
        "${NODE_MODULES_DIR}/react-native/ReactCommon/callinvoker"
        "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi"
        "${CMAKE_SOURCE_DIR}/../include")

add_library(react-native-webgpu SHARED ${SHARED_SOURCE} ${ANDROID_SOURCE})

add_library(wgpu_native SHARED IMPORTED)
set_target_properties(
        wgpu_native
        PROPERTIES IMPORTED_LOCATION
        "${CMAKE_SOURCE_DIR}/../bin/${ANDROID_ABI}/libwgpu_native.a")

target_link_libraries(
        react-native-webgpu
        wgpu_native
        ReactAndroid::jsi
        android
        ReactAndroid::reactnativejni
        fbjni::fbjni
        log)