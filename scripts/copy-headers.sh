OUT_DIR="packages/react-native-webgpu/include"
mkdir -p "$OUT_DIR"
cp submodules/wgpu-native/ffi/wgpu.h \
   submodules/wgpu-native/ffi/webgpu-headers/webgpu.h \
   "$OUT_DIR"
