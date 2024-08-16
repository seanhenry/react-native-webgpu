# Examples

You can run the examples using the instructions below.

1. Clone the project

```bash
git clone https://github.com/seanhenry/react-native-webgpu.git
cd examples/Example
```

2. Install JavaScript dependencies

```bash
yarn
```

3. Download WebGPU dependencies

```bash
wget https://github.com/seanhenry/react-native-webgpu/releases/download/v0.0.0/Bin+Headers.zip
unzip Bin+Headers.zip -d ../../packages/react-native-webgpu
```

<details>
<summary>Manual instructions</summary>

- Download `Bin+Headers.zip` from the [release page](https://github.com/seanhenry/react-native-webgpu/releases/tag/v0.0.0).
- Unzip and move the `bin` and `include` folders to `../../packages/react-native-webgpu`
</details>

4. Install pods (iOS only)

```bash
cd ios
pod install
cd ..
```

5. Build and run

```bash
yarn ios
yarn android
```
