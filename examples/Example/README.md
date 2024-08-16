# Examples

You can run the examples using the instructions below.

1. Clone

```bash
git clone https://github.com/seanhenry/react-native-webgpu.git
cd examples/Example
```

2. Install dependencies

```bash
yarn
```

3. Download `Bin+Headers.zip` from the [release page](https://github.com/seanhenry/react-native-webgpu/releases/tag/v0.0.0).
4. Unzip and move the `bin` and `include` to `packages/react-native-webgpu`
5. Install examples dependencies (iOS only)

```bash
cd ios
pod install
cd ..
```

6. Build and run

```bash
yarn ios
yarn android
```
