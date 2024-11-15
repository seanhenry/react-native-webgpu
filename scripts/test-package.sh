#!/usr/bin/env bash

SKIP_PACK=0
BUILD_DEBUG=0
BUILD_RELEASE=1
BUILD_IOS=1
BUILD_ANDROID=1
BUILD_NEW_ARCH=1
BUILD_OLD_ARCH=1

VERSION="$(jq -r '.version' packages/react-native-webgpu/package.json)"
IOS_DESTINATION="OS=18.0,name=iPhone 16"

set -e

exec 3>&1
function print() {
  echo "$1" >&3
}
exec &> "test-package.log"

[[ -s "$HOME/.rvm/scripts/rvm" ]] && source "$HOME/.rvm/scripts/rvm"

PRODUCTS_DIR="$(pwd)/products"
rm -rf "${PRODUCTS_DIR}"
mkdir -p "${PRODUCTS_DIR}"

pushd packages/react-native-webgpu

if [[ "$SKIP_PACK" != "1" ]]; then
  print "Packing react-native-webgpu"
  rm -f "react-native-webgpu-${VERSION}.tgz"
  yarn tsc
  npm pack
fi

popd

pushd examples/Example

function clean_native() {
  print "Cleaning native code"
  pushd android
  ./gradlew clean
  popd
  rm -rf 'ios/Pods' 'ios/build' 'android/build' 'android/app/.cxx' 'android/app/build'
}

function build_ios() {
  configuration="$1"
  xcodebuild -workspace Example.xcworkspace -scheme Example -configuration "$configuration" -destination "${IOS_DESTINATION}" -derivedDataPath build build | xcpretty
}

if [[ "$SKIP_PACK" != "1" ]]; then
  print "Installing react-native-webgpu from react-native-webgpu-${VERSION}.tgz package"
  yarn remove react-native-webgpu
  rm -rf node_modules
  # Important, yarn caches local .tgz so delete it from the cache
  rm -f "$(yarn config get cacheFolder)"/react-native-webgpu*
  rm -f "$(yarn config get globalFolder)"/cache/react-native-webgpu*
  yarn add "../../packages/react-native-webgpu/react-native-webgpu-${VERSION}.tgz"
fi

if [[ "$BUILD_OLD_ARCH" == "1" ]]; then
  clean_native

  if [[ "$BUILD_IOS" == "1" ]]; then
    pushd ios
    print "Installing pods for oldarch"
    bundle exec pod install
    if [[ "$BUILD_DEBUG" == "1" ]]; then
      print "Building iOS oldarch Debug"
      build_ios Debug
      mv build/Build/Products/Debug-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Debug-oldarch.app"
    fi
    if [[ "$BUILD_RELEASE" == "1" ]]; then
      print "Building iOS oldarch Release"
      build_ios Release
      mv build/Build/Products/Release-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Release-oldarch.app"
    fi
    popd
  fi

  if [[ "$BUILD_ANDROID" == "1" ]]; then
    pushd android
    if [[ "$BUILD_DEBUG" == "1" ]]; then
      print "Building Android oldarch Debug"
      ./gradlew assembleDebug -PnewArchEnabled=false
      mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-oldarch.apk"
    fi
    if [[ "$BUILD_RELEASE" == "1" ]]; then
      print "Building Android oldarch Release"
      ./gradlew assembleRelease -PnewArchEnabled=false
      mv app/build/outputs/apk/release/app-release.apk "${PRODUCTS_DIR}/Example-Release-oldarch.apk"
    fi
    popd
  fi
fi

if [[ "$BUILD_NEW_ARCH" == "1" ]]; then
  clean_native

  if [[ "$BUILD_IOS" == "1" ]]; then
    pushd ios
    print "Installing pods for newarch"
    RCT_NEW_ARCH_ENABLED=1 bundle exec pod install
    if [[ "$BUILD_DEBUG" == "1" ]]; then
      print "Building iOS newarch Debug"
      build_ios Debug
      mv build/Build/Products/Debug-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Debug-newarch.app"
    fi
    if [[ "$BUILD_RELEASE" == "1" ]]; then
      print "Building iOS newarch Release"
      build_ios Release
      mv build/Build/Products/Release-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Release-newarch.app"
    fi
    popd
  fi

  if [[ "$BUILD_ANDROID" == "1" ]]; then
    pushd android
    if [[ "$BUILD_DEBUG" == "1" ]]; then
      print "Building Android newarch Debug"
      ./gradlew assembleDebug -PnewArchEnabled=true
      mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-newarch.apk"
    fi
    if [[ "$BUILD_RELEASE" == "1" ]]; then
      print "Building Android newarch Release"
      ./gradlew assembleRelease -PnewArchEnabled=true
      mv app/build/outputs/apk/release/app-release.apk "${PRODUCTS_DIR}/Example-Release-newarch.apk"
    fi
    popd
  fi
fi

popd

print "Products moved to ${PRODUCTS_DIR}"
