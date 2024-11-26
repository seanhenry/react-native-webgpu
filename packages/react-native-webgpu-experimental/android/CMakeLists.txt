cmake_minimum_required(VERSION 3.4.1)
project(react-native-webgpu-experimental)

set(CMAKE_CXX_STANDARD 17)
set(WGPU_EXP_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(WGPU_DIR "${WGPU_EXP_DIR}/../react-native-webgpu")
set(RN_DIR "${REACT_ANDROID_DIR}/..")

find_package(fbjni REQUIRED CONFIG)
find_package(ReactAndroid REQUIRED CONFIG)
find_library(zlib z REQUIRED)

file(GLOB SHARED_SOURCE "${WGPU_EXP_DIR}/cxx/*.cpp")
file(GLOB ANDROID_SOURCE "${WGPU_EXP_DIR}/cxx/android/*.cpp")

add_library(react-native-webgpu-experimental SHARED ${SHARED_SOURCE} ${ANDROID_SOURCE})

target_include_directories(react-native-webgpu-experimental PUBLIC
        "${WGPU_EXP_DIR}/cxx"
        "${WGPU_EXP_DIR}/cxx/android"
        "${REACT_ANDROID_DIR}/src/main/jni/react/turbomodule"
        "${RN_DIR}/ReactCommon/callinvoker"
        "${RN_DIR}/ReactCommon/jsi"
        "${WGPU_DIR}/cxx"
        "${WGPU_DIR}/cxx/android"
        "${WGPU_DIR}/include"
)

# Note, REACTNATIVE_MERGED_SO is not defined on old architecture
if (REACTNATIVE_MERGED_SO OR ReactAndroid_VERSION_MINOR GREATER_EQUAL 76)
    target_link_libraries(react-native-webgpu-experimental ReactAndroid::reactnative)
else ()
    target_link_libraries(react-native-webgpu-experimental ReactAndroid::reactnativejni)
endif ()

target_link_libraries(react-native-webgpu-experimental
        android
        ReactAndroid::jsi
        fbjni::fbjni
        log
        z
        mediandk)

target_compile_options(react-native-webgpu-experimental PRIVATE
        -DLOG_TAG=\"ReactNative\"
        -fexceptions
        -frtti
        -std=c++17
        -Wall
        -Wuninitialized
)

if(NOT DEFINED IS_OLD_ARCHITECTURE)
    target_link_libraries(react-native-webgpu-experimental react_codegen_WebgpuExperimentalSpec)
    add_definitions(-DWGPU_NEW_ARCHITECTURE=1)
endif ()

