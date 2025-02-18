#!/usr/bin/env bash

SKIP_PACK=0
BUILD_DEBUG=0
BUILD_RELEASE=1
BUILD_IOS=1
BUILD_ANDROID=1
BUILD_NEW_ARCH=1
BUILD_OLD_ARCH=1
RN_VERSIONS=("0.77.1" "0.76.7")

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
VERSION="$(jq -r '.version' packages/react-native-webgpu/package.json)"
IOS_DESTINATION="OS=18.0,name=iPhone 16"

export GRADLE_OPTS="-Xmx4g -XX:MaxMetaspaceSize=1g"

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

if [[ "$SKIP_PACK" != "1" ]]; then
  # Important, yarn caches local .tgz so delete it from the cache
  print "Clearing webgpu packages from yarn caches"
  rm -f "$(yarn config get cacheFolder)"/react-native-webgpu*
  rm -f "$(yarn config get globalFolder)"/cache/react-native-webgpu*
fi

pushd packages/react-native-webgpu

if [[ "$SKIP_PACK" != "1" ]]; then
  print "Packing react-native-webgpu"
  rm -f "react-native-webgpu-${VERSION}.tgz"
  npm pack
fi

popd # packages/react-native-webgpu

pushd packages/react-native-webgpu-experimental

if [[ "$SKIP_PACK" != "1" ]]; then
  print "Packing react-native-webgpu-experimental"
  rm -f "react-native-webgpu-experimental-0.0.1.tgz"
  npm pack
fi

popd # packages/react-native-webgpu-experimental

pushd packages/react-native-webgpu-three

if [[ "$SKIP_PACK" != "1" ]]; then
  print "Packing react-native-webgpu-three"
  rm -f "react-native-webgpu-three-0.0.1.tgz"
  npm pack
fi

popd # packages/react-native-webgpu-three

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

  print "Installing packages for react native ${RN_VERSION}"
  yarn set version 3.6.4
  touch yarn.lock
  "${SCRIPT_DIR}/dependencies/${RN_VERSION}.sh"

  print "Installing webgpu packages"
  yarn add "${PACKAGES_DIR}/react-native-webgpu-experimental/react-native-webgpu-experimental-0.0.1.tgz" \
    "${PACKAGES_DIR}/react-native-webgpu/react-native-webgpu-${VERSION}.tgz" \
    "${PACKAGES_DIR}/react-native-webgpu-three/react-native-webgpu-three-0.0.1.tgz"

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
        ./gradlew --stop
        ./gradlew assembleDebug -PnewArchEnabled=false
        mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-oldarch-${RN_VERSION}.apk"
      fi
      if [[ "$BUILD_RELEASE" == "1" ]]; then
        print "Building Android oldarch Release"
        ./gradlew --stop
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
        ./gradlew --stop
        ./gradlew assembleDebug -PnewArchEnabled=true
        mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-newarch-${RN_VERSION}.apk"
      fi
      if [[ "$BUILD_RELEASE" == "1" ]]; then
        print "Building Android newarch Release"
        ./gradlew --stop
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
