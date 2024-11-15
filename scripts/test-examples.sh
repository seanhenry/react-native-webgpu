#!/usr/bin/env bash

set -e

ARCHS=(newarch oldarch)
RUN_IOS=1
RUN_ANDROID=1

function print_usage() {
  echo "Usage: $0 [-haino]"
  echo ""
  echo "Options:"
  echo "  -h  Show this help message"
  echo "  -a  Android only"
  echo "  -i  iOS only"
  echo "  -n  New architecture only"
  echo "  -o  Old architecture only"
}

while getopts "haino" opt; do
  case $opt in
    h)
      print_usage
      exit 0
      ;;
    a)
      RUN_IOS=0
      RUN_ANDROID=1
      ;;
    i)
      RUN_IOS=1
      RUN_ANDROID=0
      ;;
    n)
      ARCHS=(newarch)
      ;;
    o)
      ARCHS=(oldarch)
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      print_usage
      exit 1
      ;;
  esac
done

if [[ "$RUN_IOS" == "1" && "$RUN_ANDROID" == "1" ]]; then
  echo "Running iOS and Android on architectures: ${ARCHS[*]}"
elif [[ "$RUN_IOS" == "1" ]]; then
  echo "Running iOS on architectures: ${ARCHS[*]}"
elif [[ "$RUN_ANDROID" == "1" ]]; then
  echo "Running Android on architectures: ${ARCHS[*]}"
fi

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

IOS_BUNDLE_ID=react-native-webgpu.example
ANDROID_BUNDLE_ID=com.example
OUT_DIR="screenshots-$(date +"%Y%m%d-%H%M%S")"
mkdir -p "${OUT_DIR}/ios/newarch" "${OUT_DIR}/ios/oldarch" "${OUT_DIR}/android/newarch" "${OUT_DIR}/android/oldarch"

for ARCH in "${ARCHS[@]}"; do
  IOS_APP="products/Example-Release-${ARCH}.app"
  ANDROID_APP="products/Example-Release-${ARCH}.apk"

  if [[ "$RUN_IOS" == "1" ]]; then
    xcrun simctl install booted "${IOS_APP}"

    for EXAMPLE in "${EXAMPLES[@]}"; do
      xcrun simctl launch booted "${IOS_BUNDLE_ID}" -example "${EXAMPLE}"
      sleep 1;
      xcrun simctl io booted screenshot "${OUT_DIR}/ios/${ARCH}/${EXAMPLE}.png"
      xcrun simctl terminate booted "${IOS_BUNDLE_ID}"
    done
  fi

  if [[ "$RUN_ANDROID" == "1" ]]; then

    adb install -r "${ANDROID_APP}"

    for EXAMPLE in "${EXAMPLES[@]}"; do
      adb shell am start -n "${ANDROID_BUNDLE_ID}/.MainActivity" --es "example" "${EXAMPLE}"
      sleep 1;
      adb exec-out screencap -p > "${OUT_DIR}/android/${ARCH}/${EXAMPLE}.png"
      adb shell am force-stop "${ANDROID_BUNDLE_ID}"
    done
  fi

done

pushd "${OUT_DIR}" || exit 1

../scripts/gen-examples-html.js > "index.html"

popd || exit 1

echo "Screenshots written to $(pwd)/${OUT_DIR}"
open "$(pwd)/${OUT_DIR}/index.html"
