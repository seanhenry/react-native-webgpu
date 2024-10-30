#!/usr/bin/env bash

pushd packages/react-native-webgpu

zip Bin+Headers.zip -r bin include

popd
