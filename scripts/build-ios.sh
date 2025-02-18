#!/bin/bash -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export WGPU_NATIVE_VERSION="v22.1.0.5"

pushd submodules/wgpu-native

out_dir="${SCRIPT_DIR}/../packages/react-native-webgpu/bin"
targets=(aarch64-apple-ios aarch64-apple-ios-sim x86_64-apple-ios)
out_framework_dir="${out_dir}/wgpu_native.xcframework"

rm -rf "${out_framework_dir:?}"

for target in "${targets[@]}"; do
  rustup target add "$target"
  RUSTFLAGS="-C embed-bitcode=yes -C lto=fat -C codegen-units=1 -C panic=abort -C opt-level=3" cargo build --release --target "$target"

  pushd "target/${target}/release"
  mv libwgpu_native.a libwgpu_native_bitcode.a
  xcrun bitcode_strip libwgpu_native_bitcode.a -r -o libwgpu_native.a
  popd
done

mkdir -p 'sim-fat'
lipo "target/${targets[1]}/release/libwgpu_native.a" "target/${targets[2]}/release/libwgpu_native.a" -create -output "sim-fat/libwgpu_native.a"

wgpu_header="ffi/wgpu.h"
webgpu_header="ffi/webgpu-headers/webgpu.h"

xcodebuild -create-xcframework \
 -library "target/${targets[0]}/release/libwgpu_native.a" -headers "$wgpu_header" -headers "$webgpu_header" \
 -library "sim-fat/libwgpu_native.a" -headers "$wgpu_header" -headers "$webgpu_header" \
 -output "$out_framework_dir"

rm -rf 'sim-fat'

popd
