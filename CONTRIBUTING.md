# Contributing

## Setup

### Development OS

Please note that this project is only run using macOS so scripts will likely not work on other operating systems.

### Bash

Some scripts may contain feature unavailable to the system version of bash. Install a newer version using Homebrew:

```shell
brew install bash
```

## Manually building wgpu dependencies

[wgpu-native](https://github.com/gfx-rs/wgpu-native) is the underlying library used for WebGPU.

- [Install Rust](https://www.rust-lang.org/tools/install)
- Install submodules

```shell
git submodule update --init --recursive
```

- Build libraries

```shell
./scripts/build-android.sh
./scripts/build-ios.sh
```

- Copy headers

```shell
./scripts/copy-headers.sh
```

## Formatting native code

Please use `./scripts/format-cxx.sh` to format c++ and Objective-C

## Testing

### Examples

All `react-native-webgpu` features have been built by providing an example to prove the feature works, and to prevent regressions. Please provide a working example when adding a new feature.

### Check package

To test `react-native-webgpu` is packaged correctly, run `./scripts/test-package.sh`. This will make a local npm package and build the example app for iOS, and Android on both architectures.

### Check examples

You can run all examples using `./scripts/test-examples.sh`. Check the generated screenshots for regressions.

This script requires that you build the examples first using `./scripts/test-package.sh`
