cmake_minimum_required(VERSION 3.4.1)
project(react-native-webgpu)

set(CMAKE_CXX_STANDARD 17)
set(WGPU_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(RN_DIR "${REACT_ANDROID_DIR}/..")

find_package(fbjni REQUIRED CONFIG)
find_package(ReactAndroid REQUIRED CONFIG)

file(GLOB SHARED_SOURCE "${WGPU_DIR}/cxx/*.cpp")
file(GLOB ANDROID_SOURCE "${WGPU_DIR}/cxx/android/*.cpp")

add_library(react-native-webgpu SHARED ${SHARED_SOURCE} ${ANDROID_SOURCE})

target_include_directories(react-native-webgpu PUBLIC
        "${WGPU_DIR}/cxx"
        "${WGPU_DIR}/cxx/android"
        "${WGPU_DIR}/include"
        "${REACT_ANDROID_DIR}/src/main/jni/react/turbomodule"
        "${RN_DIR}/ReactCommon/callinvoker"
        "${RN_DIR}/ReactCommon/jsi"
)

add_library(wgpu_native SHARED IMPORTED)
set_target_properties(wgpu_native
        PROPERTIES IMPORTED_LOCATION
        "${WGPU_DIR}/bin/${ANDROID_ABI}/libwgpu_native.a")

# Note, REACTNATIVE_MERGED_SO is not defined on old architecture
if (REACTNATIVE_MERGED_SO OR ReactAndroid_VERSION_MINOR GREATER_EQUAL 76)
    target_link_libraries(react-native-webgpu ReactAndroid::reactnative)
else ()
    target_link_libraries(react-native-webgpu ReactAndroid::reactnativejni)
endif ()

target_link_libraries(react-native-webgpu
        wgpu_native
        android
        ReactAndroid::jsi
        fbjni::fbjni
        log)

target_compile_options(react-native-webgpu PRIVATE
        -DLOG_TAG=\"ReactNative\"
        -fexceptions
        -frtti
        -std=c++17
        -Wall
        -Wuninitialized
)

if(NOT DEFINED IS_OLD_ARCHITECTURE)
    target_link_libraries(react-native-webgpu react_codegen_WebgpuSpec)
    add_definitions(-DWGPU_NEW_ARCHITECTURE=1)
endif ()
