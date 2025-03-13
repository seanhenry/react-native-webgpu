#!/usr/bin/env bash

# Note, >0.81.0 introduces "_interopRequireDefault is not a function" error when `unstable_enablePackageExports` is `true` in metro.config.js (required for three.js)
npm pkg set resolutions.metro=0.81.0
npm pkg set resolutions.metro-config=0.81.0
npm pkg set resolutions.metro-core=0.81.0
npm pkg set resolutions.metro-resolver=0.81.0
npm pkg set resolutions.metro-runtime=0.81.0
npm pkg set resolutions.metro-source-map=0.81.0

yarn add "@gltf-transform/core@4.0.10" \
  "@react-navigation/native@7.0.14" \
  "@react-navigation/native-stack@7.2.0" \
  "lodash@4.17.21" \
  "react-native-gesture-handler@2.23.1" \
  "react-native-launch-arguments@4.0.4" \
  "react-native-reanimated@3.16.7" \
  "react-native-safe-area-context@5.2.0" \
  "react-native-screens@4.7.0" \
  "teapot@1.0.0" \
  "three@0.166.1" \
  "wgpu-matrix@3.0.1" \
  "@babel/plugin-transform-export-namespace-from@7.24.7" \
  "@types/lodash@4.17.7" \
  "@types/three@0.166.0" \
  "fast-text-encoding@1.0.6"
