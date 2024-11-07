#!/usr/bin/env bash

set -e

IOS_APP="products/Example-Release-oldarch.app"
ANDROID_APP="products/Example-Release-oldarch.apk"
IOS_BUNDLE_ID=react-native-webgpu.example
ANDROID_BUNDLE_ID=com.example

EXAMPLES=(
 'HelloTriangle'
 'HelloTriangleMSAA'
 'RotatingCube'
 'TwoCubes'
 'TexturedCube'
 'InstancedCube'
 'FractalCube'
 'CubeMap'
 # GPGPU
 'ComputeBoids'
 'GameOfLife'
 'BitonicSort'
 # WebGPUFeatures
 'SamplerParameters'
 'ReversedZ'
 'RenderBundles'
 'OcclusionQueries'
 # GraphicsTechniques
 'Cameras'
 'NormalMap'
 'ShadowMapping'
 'DeferredRendering'
 'ParticlesHDR'
 'ImageBlur'
 'Cornell'
 'ABuffer'
 'SkinnedMesh'
 'VolumeRendering'
 'Wireframe'
 # Three
# 'HelloThree'
# 'Clearcoat'
# 'Clipping'
# # Camera
# 'LogarithmicDepthBuffer'
# # Compute
# 'ComputeGeometry'
# 'ComputeParticles'
# # Loader
# 'GLTFLoader'
 # InHouse
 'FlakesTexture'
 'Crop'
 'Video'
 'Outlines'
 'Portal'
# 'Thread'
# 'AdapterInfo'
# 'ExceedDeviceLimits'
 'CWTriangle'
 'ClearBuffer'
)

rm -rf screenshots
mkdir -p screenshots/ios screenshots/android

xcrun simctl install booted "${IOS_APP}"

for EXAMPLE in "${EXAMPLES[@]}"; do
  xcrun simctl launch booted "${IOS_BUNDLE_ID}" -example "${EXAMPLE}"
  sleep 1;
  xcrun simctl io booted screenshot "screenshots/ios/${EXAMPLE}.png"
  xcrun simctl terminate booted "${IOS_BUNDLE_ID}"
done

adb install -r "${ANDROID_APP}"

for EXAMPLE in "${EXAMPLES[@]}"; do
  adb shell am start -n "${ANDROID_BUNDLE_ID}/.MainActivity" --es "example" "${EXAMPLE}"
  sleep 1;
  adb exec-out screencap -p > "screenshots/android/${EXAMPLE}.png"
  adb shell am force-stop "${ANDROID_BUNDLE_ID}"
done
