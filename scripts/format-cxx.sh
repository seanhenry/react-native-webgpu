#!/usr/bin/env bash

shopt -s globstar

function format_files {
  for file in $1; do
    clang-format -style=file:packages/react-native-webgpu/.clang-format -i "$file"
  done
}

format_files "packages/react-native-webgpu/cxx/**/*.h"
format_files "packages/react-native-webgpu/cxx/**/*.cpp"
format_files "packages/react-native-webgpu/cxx/**/*.m"
format_files "packages/react-native-webgpu/cxx/**/*.mm"

format_files "packages/react-native-webgpu-experimental/cxx/**/*.h"
format_files "packages/react-native-webgpu-experimental/cxx/**/*.cpp"
#format_files "packages/react-native-webgpu-experimental/cxx/**/*.m"
format_files "packages/react-native-webgpu-experimental/cxx/**/*.mm"
