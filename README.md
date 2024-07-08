# react-native-webgpu

Coming soon. A [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) port for react native which aims to provide direct access to Metal and Vulkan for iOS and Android, and support for libraries like [Three.js](https://threejs.org).

Interested in this project? Star it in GitHub to show your support.

Dependent projects:
- [react-native-webgpu-three](https://github.com/seanhenry/react-native-webgpu-three)

## Examples

If you want to check the progress of this project, or play around with what's been implemented so far, you can check out [the examples](examples/ExampleRN)

## Installation

*Coming soon*

## Contributing

See the [contributing guide](CONTRIBUTING.md) to learn how to contribute to the repository and the development workflow.

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)

## Build locally

### Build C bindings

```shell
git submodule update --init --recursive
./scripts/build-android.sh
./scripts/build-ios.sh
```
