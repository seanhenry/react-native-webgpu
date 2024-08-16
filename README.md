# React Native WebGPU

Coming soon. A [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) port for react native which aims to provide direct access to Metal and Vulkan for iOS and Android, and support for libraries like [Three.js](https://threejs.org).

Check the subpackages for instructions:
- [react-native-webgpu](packages/react-native-webgpu)

Direct access to the low-level graphics api [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) in React Native.

- [react-native-webgpu-three](packages/react-native-webgpu-three)

A small helper library to enable [Three.js](https://threejs.org/) projects on React Native.

## Running the examples

You can find instructions to run the examples [here](examples/Example).

## Manually building wgpu dependencies

Looking to build the examples? You don't need to manually build the dependencies for that. See [react-native-webgpu](examples/Example) for instructions instead.

- [Install Rust](https://www.rust-lang.org/tools/install)
- Install submodules

```shell
git submodule update --init --recursive
```

- Build libraries and copy

```shell
./scripts/build-android.sh
./scripts/build-ios.sh
```

- Copy headers

```shell
./scripts/copy-headers.sh
```







