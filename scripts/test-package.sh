#!/usr/bin/env bash

[[ -s "$HOME/.rvm/scripts/rvm" ]] && source "$HOME/.rvm/scripts/rvm"

VERSION="$(jq -r '.version' packages/react-native-webgpu/package.json)"
IOS_DESTINATION="OS=18.0,name=iPhone 16"

set -x
set -e

PRODUCTS_DIR="$(pwd)/products"
rm -rf "${PRODUCTS_DIR}"
mkdir -p "${PRODUCTS_DIR}"

pushd packages/react-native-webgpu || exit 1

rm -f "react-native-webgpu-${VERSION}.tgz"
npm pack

popd || exit 1

pushd examples/Example || exit 1

function clean_native() {
  pushd android || exit 1
  ./gradlew clean
  popd || exit 1
  rm -rf 'ios/Pods' 'ios/build' 'android/build' 'android/app/.cxx' 'android/app/build'
}

function build_ios() {
  configuration="$1"
  xcodebuild -workspace Example.xcworkspace -scheme Example -configuration "$configuration" -destination "${IOS_DESTINATION}" -derivedDataPath build build | xcpretty
}

yarn remove react-native-webgpu
rm -rf node_modules
yarn add "../../packages/react-native-webgpu/react-native-webgpu-${VERSION}.tgz"

clean_native

pushd ios || exit 1
bundle exec pod install
#build_ios Debug
#mv build/Build/Products/Debug-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Debug-oldarch.app"
build_ios Release
mv build/Build/Products/Release-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Release-oldarch.app"
popd || exit 1

pushd android || exit 1
#./gradlew assembleDebug -PnewArchEnabled=false
#mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-oldarch.apk"
./gradlew assembleRelease -PnewArchEnabled=false
mv app/build/outputs/apk/release/app-release.apk "${PRODUCTS_DIR}/Example-Release-oldarch.apk"
popd || exit 1

clean_native

pushd ios || exit 1
RCT_NEW_ARCH_ENABLED=1 bundle exec pod install
#build_ios Debug
#mv build/Build/Products/Debug-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Debug-newarch.app"
build_ios Release
mv build/Build/Products/Release-iphonesimulator/Example.app "${PRODUCTS_DIR}/Example-Release-newarch.app"
popd || exit 1

pushd android || exit 1
#./gradlew assembleDebug -PnewArchEnabled=true
#mv app/build/outputs/apk/debug/app-debug.apk "${PRODUCTS_DIR}/Example-Debug-newarch.apk"
./gradlew assembleRelease -PnewArchEnabled=true
mv app/build/outputs/apk/release/app-release.apk "${PRODUCTS_DIR}/Example-Release-newarch.apk"
popd || exit 1

popd || exit 1
