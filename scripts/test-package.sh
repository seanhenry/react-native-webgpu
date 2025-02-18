#!/usr/bin/env bash

SKIP_PACK=0
BUILD_DEBUG=0
BUILD_RELEASE=1
BUILD_IOS=1
BUILD_ANDROID=1
BUILD_NEW_ARCH=1
BUILD_OLD_ARCH=1
RN_VERSIONS=("0.75.4" "0.76.2")

VERSION="$(jq -r '.version' packages/react-native-webgpu/package.json)"
IOS_DESTINATION="OS=18.0,name=iPhone 16"

set -e
set -E

mkdir -p ".test"

function print() {
  echo "$1" >&3
}

TEST_DIR="$(pwd)/.test"
PRODUCTS_DIR="${TEST_DIR}/products"
LOG_FILE="${TEST_DIR}/test-package.log"

rm -f "${LOG_FILE}"
exec 3>&1 4>&2
exec &> "${LOG_FILE}"

function on_err() {
  exec 1>&3 2>&4
  tail "${LOG_FILE}"
  echo "See full log at ${LOG_FILE}"
}

trap on_err ERR

[[ -s "$HOME/.rvm/scripts/rvm" ]] && source "$HOME/.rvm/scripts/rvm"

if [ -d "${PRODUCTS_DIR}" ]; then
  mv "${PRODUCTS_DIR}" ~/.Trash/"products-$(date +"%Y-%m-%d-%H.%M.%S")"
fi
mkdir -p "${PRODUCTS_DIR}"

pushd packages/react-native-webgpu

if [[ "$SKIP_PACK" != "1" ]]; then
  print "Packing react-native-webgpu"
  rm -f "react-native-webgpu-${VERSION}.tgz"
  # Important, yarn caches local .tgz so delete it from the cache
  rm -f "$(yarn config get cacheFolder)"/react-native-webgpu*
  rm -f "$(yarn config get globalFolder)"/cache/react-native-webgpu*
  npm pack
fi

popd # packages/react-native-webgpu

function clean_native() {
  print "Cleaning native code"
  pushd android
  ./gradlew clean
  popd # android
  rm -rf 'ios/Pods' 'ios/build' 'android/build' 'android/app/.cxx' 'android/app/build'
}

function build_ios() {
  configuration="$1"
  xcodebuild -workspace Example.xcworkspace -scheme Example -configuration "$configuration" -destination "${IOS_DESTINATION}" -derivedDataPath build build | xcpretty
}

for RN_VERSION in "${RN_VERSIONS[@]}"; do
  rm -rf "${TEST_DIR:?}/${RN_VERSION}"
  mkdir -p "${TEST_DIR}/${RN_VERSION}"

  pushd "${TEST_DIR}/${RN_VERSION}"

  npx -y @react-native-community/cli@latest init Example --version "${RN_VERSION}" --skip-install

  # Required because >= operator not supported for rvm
  cp "../../examples/Example/Gemfile" "Example"

  pushd Example

  print "Generating examples project for react-native@${RN_VERSION}"
  PACKAGES_DIR="../../../packages"
  EXAMPLE_PROJ="../../../examples/Example"

  print "Installing dependencies"
  touch yarn.lock
  yarn add "@gltf-transform/core@^4.0.10" \
    "@react-navigation/native@^7.0.3" \
    "@react-navigation/native-stack@^7.1.0" \
    "lodash@^4.17.21" \
    "react-native-gesture-handler@^2.21.2" \
    "react-native-launch-arguments@^4.0.2" \
    "react-native-reanimated@^3.16.2" \
    "react-native-safe-area-context@^4.14.0" \
    "react-native-screens@^4.2.0" \
    "teapot@^1.0.0" \
    "three@0.166.1" \
    "wgpu-matrix@^3.0.1" \
    "@babel/plugin-transform-export-namespace-from@^7.24.7" \
    "@types/lodash@^4.17.7" \
    "@types/three@^0.166.0" \
    "fast-text-encoding@^1.0.6" \
    "${PACKAGES_DIR}/react-native-webgpu-experimental" \
    "${PACKAGES_DIR}/react-native-webgpu-three" \
    "${PACKAGES_DIR}/react-native-webgpu/react-native-webgpu-${VERSION}.tgz"

  print "Copying config and source"
  cp -r "${EXAMPLE_PROJ}/babel.config.js" \
    "${EXAMPLE_PROJ}/index.js" \
    "${EXAMPLE_PROJ}/metro.config.js" \
    "${EXAMPLE_PROJ}/tsconfig.json" \
    "${EXAMPLE_PROJ}/.eslintignore" \
    "${EXAMPLE_PROJ}/.eslintrc.js" \
    "${EXAMPLE_PROJ}/.prettierignore" \
    "${EXAMPLE_PROJ}/.prettierrc.js" \
    "${EXAMPLE_PROJ}/babel" \
    "${EXAMPLE_PROJ}/src" \
    .

  print "Changing node_modules relative path in tsconfig.json"
  sed -i '' 's/..\/..\/node_modules/node_modules/g' "tsconfig.json"

  print "Setting android minSdkVersion to 27"
  sed -E -i '' 's/minSdkVersion = [0-9]+/minSdkVersion = 27/' "android/build.gradle"

  print "Installing ruby dependencies"
  bundle

  print "Checking for TS errors"
  yarn tsc --noEmit

  print "Linting"
  yarn eslint .

  if [[ "$BUILD_OLD_ARCH" == "1" ]]; then
    clean_native

    if [[ "$BUILD_IOS" == "1" ]]; then
      pushd ios
      print "Installing pods for oldarch"
      RCT_NEW_ARCH_ENABLED=0 bundle exec pod update
      if [[ "$BUILD_DEBUG" == "1" ]]; then
        print "Building iOS oldarch Debug"
        build_ios Debug
        mv build/Build/Products/Debug-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Debug-oldarch-${RN_VERSION}.app"
      fi
      if [[ "$BUILD_RELEASE" == "1" ]]; then
        print "Building iOS oldarch Release"
        build_ios Release
        mv build/Build/Products/Release-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Release-oldarch-${RN_VERSION}.app"
      fi
      popd # ios
    fi

    if [[ "$BUILD_ANDROID" == "1" ]]; then
      pushd android
      if [[ "$BUILD_DEBUG" == "1" ]]; then
        print "Building Android oldarch Debug"
        ./gradlew assembleDebug -PnewArchEnabled=false
        mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-oldarch-${RN_VERSION}.apk"
      fi
      if [[ "$BUILD_RELEASE" == "1" ]]; then
        print "Building Android oldarch Release"
        ./gradlew assembleRelease -PnewArchEnabled=false
        mv app/build/outputs/apk/release/app-release.apk "${PRODUCTS_DIR}/Example-Release-oldarch-${RN_VERSION}.apk"
      fi
      popd # android
    fi
  fi

  if [[ "$BUILD_NEW_ARCH" == "1" ]]; then
    clean_native

    if [[ "$BUILD_IOS" == "1" ]]; then
      pushd ios
      print "Installing pods for newarch"
      RCT_NEW_ARCH_ENABLED=1 bundle exec pod update
      if [[ "$BUILD_DEBUG" == "1" ]]; then
        print "Building iOS newarch Debug"
        build_ios Debug
        mv build/Build/Products/Debug-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Debug-newarch-${RN_VERSION}.app"
      fi
      if [[ "$BUILD_RELEASE" == "1" ]]; then
        print "Building iOS newarch Release"
        build_ios Release
        mv build/Build/Products/Release-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Release-newarch-${RN_VERSION}.app"
      fi
      popd # ios
    fi

    if [[ "$BUILD_ANDROID" == "1" ]]; then
      pushd android
      if [[ "$BUILD_DEBUG" == "1" ]]; then
        print "Building Android newarch Debug"
        ./gradlew assembleDebug -PnewArchEnabled=true
        mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-newarch-${RN_VERSION}.apk"
      fi
      if [[ "$BUILD_RELEASE" == "1" ]]; then
        print "Building Android newarch Release"
        ./gradlew assembleRelease -PnewArchEnabled=true
        mv app/build/outputs/apk/release/app-release.apk "${PRODUCTS_DIR}/Example-Release-newarch-${RN_VERSION}.apk"
      fi
      popd # android
    fi
  fi

  popd # Example

  popd # "${TEST_DIR}/${RN_VERSION}"

done

print "Products moved to ${PRODUCTS_DIR}"
