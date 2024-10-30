#!/bin/bash -e

pushd submodules/wgpu-native

# iOS (sim arm64)
export CARGO_BUILD_TARGET=aarch64-apple-ios-sim

rustup target add $CARGO_BUILD_TARGET
cargo build --release

# iOS (sim x86_64)
export CARGO_BUILD_TARGET=x86_64-apple-ios

rustup target add $CARGO_BUILD_TARGET
cargo build --release

# iOS (arm64)
export CARGO_BUILD_TARGET=aarch64-apple-ios

rustup target add $CARGO_BUILD_TARGET
cargo build --release

popd

OUT_DIR="packages/react-native-webgpu/bin"
TARGET_DIR="submodules/wgpu-native/target"
HEADERS_DIR="submodules/wgpu-native/ffi"

rm -rf "$OUT_DIR/ios" "$OUT_DIR/ios-simulator" "$OUT_DIR/wgpu_native.xcframework"
mkdir -p "$OUT_DIR/ios" "$OUT_DIR/ios-simulator"

lipo "$TARGET_DIR/aarch64-apple-ios-sim/release/libwgpu_native.a" "$TARGET_DIR/x86_64-apple-ios/release/libwgpu_native.a" -create -output "$OUT_DIR/ios-simulator/libwgpu_native.a"
cp "$TARGET_DIR/aarch64-apple-ios/release/libwgpu_native.a" "$OUT_DIR/ios/libwgpu_native.a"

xcodebuild -create-xcframework \
 -library "$OUT_DIR/ios-simulator/libwgpu_native.a" -headers "$HEADERS_DIR/wgpu.h" -headers "$HEADERS_DIR/webgpu-headers/webgpu.h" \
 -library "$OUT_DIR/ios/libwgpu_native.a" -headers "$HEADERS_DIR/wgpu.h" -headers "$HEADERS_DIR/webgpu-headers/webgpu.h" \
 -output "$OUT_DIR/wgpu_native.xcframework" \

rm -rf "$OUT_DIR/ios" "$OUT_DIR/ios-simulator"
