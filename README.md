# react-native-webgpu

Coming soon. A [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) port for react native which aims to provide direct access to Metal and Vulkan for iOS and Android, and support for libraries like [Three.js](https://threejs.org).

Interested in this project? Star it in GitHub to show your support.

Dependent projects:
- [react-native-webgpu-three](https://github.com/seanhenry/react-native-webgpu-three)

# Build locally

## Build C bindings

```shell
git submodule update --init --recursive
./scripts/build-android.sh
./scripts/build-ios.sh
```


