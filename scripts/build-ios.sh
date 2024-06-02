#!/bin/bash -e

cd submodules/wgpu-native

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
