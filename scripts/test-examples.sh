#!/usr/bin/env bash

set -e

ARCHS=(newarch oldarch)
RUN_IOS=1
RUN_ANDROID=1
RN_VERSIONS=("0.78.0" "0.77.1")

function print_usage() {
  echo "Usage: $0 [-haino]"
  echo ""
  echo "Options:"
  echo "  -h           Show this help message"
  echo "  -a           Android only"
  echo "  -i           iOS only"
  echo "  -n           New architecture only"
  echo "  -o           Old architecture only"
  echo "  -v rnversion Specified react native version only (76 or 77)"
}

while getopts "hainov:" opt; do
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
    v)
      if [[ "$OPTARG" == "76" ]]; then
        RN_VERSIONS=("0.76.7")
      elif [[ "$OPTARG" == "77" ]]; then
        RN_VERSIONS=("0.77.1")
      else
        echo "Invalid react-native version: -v $OPTARG" >&2
        print_usage
        exit 1
      fi
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      print_usage
      exit 1
      ;;
  esac
done

if [[ "$RUN_IOS" == "1" && "$RUN_ANDROID" == "1" ]]; then
  echo "platforms=(ios android) archs=(${ARCHS[*]}) rnversions=(${RN_VERSIONS[*]})"
elif [[ "$RUN_IOS" == "1" ]]; then
  echo "platforms=(ios) archs=(${ARCHS[*]}) rnversions=(${RN_VERSIONS[*]})"
elif [[ "$RUN_ANDROID" == "1" ]]; then
  echo "platforms=(android) archs=(${ARCHS[*]}) rnversions=(${RN_VERSIONS[*]})"
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
 'ResizeCanvas'
 'AnimateCanvas'
)

ROOT="$(pwd)"
IOS_BUNDLE_ID="org.reactjs.native.example.Example"
ANDROID_BUNDLE_ID="com.example"
OUT_DIR="${ROOT}/.test/screenshots/$(date +"%Y%m%d-%H%M%S")"
PRODUCTS_DIR="${ROOT}/.test/products"
LOG_FILE="${OUT_DIR}/examples.log"

mkdir -p "${OUT_DIR}"

RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
RESET=$(tput sgr0)

function print() {
  echo "$1" >&3
}

function print_success() {
  echo "${GREEN}${1}${RESET}" >&3
}

function print_failure() {
  echo "${RED}${1}${RESET}" >&3
}

exec 3>&1 4>&2
exec &> "${LOG_FILE}"

for RN_VERSION in "${RN_VERSIONS[@]}"; do
  mkdir -p "${OUT_DIR}/${RN_VERSION}/ios/newarch" \
    "${OUT_DIR}/${RN_VERSION}/ios/oldarch" \
    "${OUT_DIR}/${RN_VERSION}/android/newarch" \
    "${OUT_DIR}/${RN_VERSION}/android/oldarch"

  for ARCH in "${ARCHS[@]}"; do
    IOS_APP="${PRODUCTS_DIR}/Example-Release-${ARCH}-${RN_VERSION}.app"
    ANDROID_APP="${PRODUCTS_DIR}/Example-Release-${ARCH}-${RN_VERSION}.apk"

    if [[ "$RUN_IOS" == "1" ]]; then
      xcrun simctl install booted "${IOS_APP}"

      for EXAMPLE in "${EXAMPLES[@]}"; do
        xcrun simctl launch booted "${IOS_BUNDLE_ID}" -example "${EXAMPLE}"
        sleep 1;
        xcrun simctl io booted screenshot "${OUT_DIR}/${RN_VERSION}/ios/${ARCH}/${EXAMPLE}.png"
        set +e
        xcrun simctl terminate booted "${IOS_BUNDLE_ID}"
        if [ $? -ne 0 ]; then
          print_failure "[Failed] ios-${RN_VERSION}-${ARCH} ${EXAMPLE}"
        else
          print_success "[Screenshot] ios-${RN_VERSION}-${ARCH} ${EXAMPLE}"
        fi
        set -e
      done
    fi

    if [[ "$RUN_ANDROID" == "1" ]]; then

      adb install -r "${ANDROID_APP}"

      for EXAMPLE in "${EXAMPLES[@]}"; do
        adb shell am start -n "${ANDROID_BUNDLE_ID}/.MainActivity" --es "example" "${EXAMPLE}"
        sleep 1;
        adb exec-out screencap -p > "${OUT_DIR}/${RN_VERSION}/android/${ARCH}/${EXAMPLE}.png"

        set +e
        adb shell pidof "${ANDROID_BUNDLE_ID}"
        if [ $? -ne 0 ]; then
          print_failure "[Failed] android-${RN_VERSION}-${ARCH} ${EXAMPLE}"
        else
          print_success "[Screenshot] android-${RN_VERSION}-${ARCH} ${EXAMPLE}"
        fi

        adb shell am force-stop "${ANDROID_BUNDLE_ID}"
        set -e
      done
    fi

  done
done

pushd "${OUT_DIR}"

"${ROOT}/scripts/gen-examples-html.js" > "index.html"

popd

print "Screenshots written to ${OUT_DIR}"
open "${OUT_DIR}/index.html"
