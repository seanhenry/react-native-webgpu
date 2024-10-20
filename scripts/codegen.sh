#!/usr/bin/env bash

pushd packages/react-native-webgpu
yarn react-native codegen
popd

./scripts/format-cxx.sh
