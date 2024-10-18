// WARNING: Example/android/app/build/generated/autolinking/autolinking.json needs deleting as it caches these values
module.exports = {
  dependency: {
    platforms: {
      android: {
        // CMakeLists.txt for specs because `includesGeneratedCode: true`
        cmakeListsPath: '../cxx/android/generated/jni/CMakeLists.txt',
        // CMakeLists.txt for our cxx code
        cxxModuleCMakeListsPath: './CMakeLists.txt',
        // The same name as add_library in CMakeLists.txt
        cxxModuleCMakeListsModuleName: 'react-native-webgpu',
        // WGPUJsi.h is imported from autolinking.cpp
        cxxModuleHeaderName: 'WGPUJsi',
      },
    },
  },
};
