#!/usr/bin/env bash

set -e

[[ -s "$HOME/.rvm/scripts/rvm" ]] && source "$HOME/.rvm/scripts/rvm"

pushd examples/Example

yarn pod:install

pushd android
./gradlew react-native-webgpu:generateCodegenArtifactsFromSchema
popd

popd
