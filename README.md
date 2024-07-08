# react-native-webgpu

Coming soon. A [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) port for react native which aims to provide direct access to Metal and Vulkan for iOS and Android, and support for libraries like [Three.js](https://threejs.org).

Interested in this project? Star it in GitHub to show your support.

Dependent projects:
- [react-native-webgpu-three](https://github.com/seanhenry/react-native-webgpu-three)

## Examples

If you want to check the progress of this project, or play around with what's been implemented so far, you can check out the examples.

1. Install library dependencies

```bash
git submodule update --init --recursive
yarn
```

2. Download the libraries from the [release page](https://github.com/seanhenry/react-native-webgpu/releases/tag/v0.0.0).
3. Create a `/bin` folder at the root of this project and move the libraries there
4. Install examples dependencies

```bash
cd examples/ExampleRN
yarn

# iOS
cd ios
pod install
cd ..

# Android coming soon
```

5. Build and run

```bash
yarn ios
```

## License

MIT

## Build locally

### Build C bindings

```shell
git submodule update --init --recursive
./scripts/build-android.sh
./scripts/build-ios.sh
```
