#!/bin/bash -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export WGPU_NATIVE_VERSION="v22.1.0.5"
export WGPU_ANDROID_NDK_VERSION="27.2.12479018"

pushd submodules/wgpu-native

rm -rf dist

out_dir="$SCRIPT_DIR/../packages/react-native-webgpu/bin"
targets=(aarch64-linux-android armv7-linux-androideabi x86_64-linux-android i686-linux-android)
triples=(aarch64-linux-android armv7a-linux-androideabi x86_64-linux-android i686-linux-android)
archs=(arm64-v8a armeabi-v7a x86_64 x86)
envs=(CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER CARGO_TARGET_ARMV7_LINUX_ANDROIDEABI_LINKER CARGO_TARGET_X86_64_LINUX_ANDROID_LINKER CARGO_TARGET_I686_LINUX_ANDROID_LINKER)

min_sdk_version=21
platform=darwin-x86_64 # Platform of local machine e.g. linux-x86_64
android_ndk_home=$ANDROID_HOME/ndk/27.2.12479018
llvm_objcopy="$android_ndk_home/toolchains/llvm/prebuilt/$platform/bin/llvm-objcopy"

for arch in "${archs[@]}"; do
  rm -rf "${out_dir:?}/$arch"
done

for i in "${!targets[@]}"; do

  target="${targets[$i]}"
  triple="${triples[$i]}"
  arch="${archs[$i]}"
  env="${envs[$i]}"
  cc="$android_ndk_home/toolchains/llvm/prebuilt/$platform/bin/${triple}${min_sdk_version}-clang"

  rustup target add "$target"

  export "$env=$cc"
  CC="$cc" \
    CXX="$cc++" \
    CLANG_PATH="$cc" \
    LLVM_CONFIG_PATH="$android_ndk_home/toolchains/llvm/prebuilt/$platform/bin/llvm-config" \
    BINDGEN_EXTRA_CLANG_ARGS="'-isysroot $android_ndk_home/toolchains/llvm/prebuilt/$platform/sysroot'" \
    RUSTFLAGS="-C embed-bitcode=yes -C lto=fat -C codegen-units=1 -C panic=abort" \
    cargo build --release --target "$target"

  "$llvm_objcopy" --strip-unneeded "target/$target/release/libwgpu_native.a"

  mkdir -p "$out_dir/$arch"
  mv "target/$target/release/libwgpu_native.a" "$out_dir/$arch"

done

popd
