#!/usr/bin/env bash

mkdir -p lib/typescript
cat types/wgsl.d.ts types/webGpuTypes.d.ts > lib/typescript/react-native-webgpu.d.ts
