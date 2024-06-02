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

# Make fat lib for simulators
mkdir -p bin
TARGET_DIR=submodules/wgpu-native/target
lipo $TARGET_DIR/aarch64-apple-ios-sim/release/libwgpu_native.a $TARGET_DIR/x86_64-apple-ios/release/libwgpu_native.a -create -output bin/libwgpu_native_iossim_x86_64_aarch64.a
cp $TARGET_DIR/aarch64-apple-ios/release/libwgpu_native.a bin/libwgpu_native_ios_aarch64.a
