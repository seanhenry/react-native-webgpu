// WARNING: Android builds cache these values so delete build directories when changing these values.
// `rm -rf android/build android/app/build`
// See android/build/generated/autolinking/autolinking.json for cached information about cxx
// See android/app/build/generated/autolinking/.../PackageList.java for cached information about java
module.exports = {
  dependency: {
    platforms: {
      android: {
        packageImportPath:
          'import com.webgpu.experimental.WebgpuExperimentalPackage;',
        packageInstance: 'new WebgpuExperimentalPackage()',
        // CMakeLists.txt for specs because `includesGeneratedCode: true`
        cmakeListsPath: '../cxx/android/generated/jni/CMakeLists.txt',
        // CMakeLists.txt for our cxx code
        cxxModuleCMakeListsPath: './CMakeLists.txt',
        // The same name as add_library in CMakeLists.txt
        cxxModuleCMakeListsModuleName: 'react-native-webgpu-experimental',
        // WGPUExperimentalJsi.h is imported from autolinking.cpp
        cxxModuleHeaderName: 'WGPUExperimentalJsi',
      },
    },
  },
};