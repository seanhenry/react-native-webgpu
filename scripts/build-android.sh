#!/bin/bash -e

pushd submodules/wgpu-native

# Taken from https://github.com/gfx-rs/wgpu-native/issues/183 and modified

MIN_SDK_VERSION=21
NDK_VERSION=25.0.8775105
PLATFORM=darwin-x86_64
# This changes depending on platform e.g. linux-x86_64

# workaround missing libgcc in ndk r23+
mkdir -p tmp-lib
echo "INPUT(-lunwind)" | tee tmp-lib/libgcc.a
export RUSTFLAGS="-L $PWD/tmp-lib"

# common
export LIBCLANG_PATH=$ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$PLATFORM/lib64/libclang.dylib # depends on NDK_VERSION
export LLVM_CONFIG_PATH=$ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$PLATFORM/bin/llvm-config
export BINDGEN_EXTRA_CLANG_ARGS="-isysroot $ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$PLATFORM/sysroot/"

# aarch64
export CLANG_PATH=$ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$PLATFORM/bin/aarch64-linux-android$MIN_SDK_VERSION-clang
export CC=$CLANG_PATH
export CXX=$CLANG_PATH++
export CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER=$CLANG_PATH
export CARGO_BUILD_TARGET=aarch64-linux-android

rustup target add $CARGO_BUILD_TARGET
cargo build --release

# armv7a
export CLANG_PATH=$ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$PLATFORM/bin/armv7a-linux-androideabi$MIN_SDK_VERSION-clang
export CC=$CLANG_PATH
export CXX=$CLANG_PATH++
export CARGO_TARGET_ARMV7_LINUX_ANDROIDEABI_LINKER=$CLANG_PATH
export CARGO_BUILD_TARGET=armv7-linux-androideabi

rustup target add $CARGO_BUILD_TARGET
cargo build --release

# x86_64
export CLANG_PATH=$ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$PLATFORM/bin/x86_64-linux-android$MIN_SDK_VERSION-clang
export CC=$CLANG_PATH
export CXX=$CLANG_PATH++
export CARGO_TARGET_X86_64_LINUX_ANDROID_LINKER=$CLANG_PATH
export CARGO_BUILD_TARGET=x86_64-linux-android

rustup target add $CARGO_BUILD_TARGET
cargo build --release

# i686
export CLANG_PATH=$ANDROID_SDK_ROOT/ndk/$NDK_VERSION/toolchains/llvm/prebuilt/$PLATFORM/bin/i686-linux-android$MIN_SDK_VERSION-clang
export CC=$CLANG_PATH
export CXX=$CLANG_PATH++
export CARGO_TARGET_I686_LINUX_ANDROID_LINKER=$CLANG_PATH
export CARGO_BUILD_TARGET=i686-linux-android

rustup target add $CARGO_BUILD_TARGET
cargo build --release

popd

function mv_lib {
  TRIPLE="$1"
  ARCH="$2"
  OUT_DIR="packages/react-native-webgpu/bin/$ARCH"
  mkdir -p "$OUT_DIR"
  mv "submodules/wgpu-native/target/$TRIPLE/release/libwgpu_native.a" "$OUT_DIR"
}

mv_lib aarch64-linux-android arm64-v8a
mv_lib armv7-linux-androideabi armeabi-v7a
mv_lib x86_64-linux-android x86_64
mv_lib i686-linux-android x86
