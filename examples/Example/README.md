# Examples

You can run the examples using the instructions below.

1. Clone the project

```bash
git clone https://github.com/seanhenry/react-native-webgpu.git
```

2. Compile react-native-webgpu

```bash
cd packages/react-native-webgpu
yarn tsc
cd ../..
```

3. Install JavaScript dependencies

```bash
cd react-native-webgpu/examples/Example
yarn
```

4. Download WebGPU dependencies

```bash
wget https://github.com/seanhenry/react-native-webgpu/releases/download/v0.0.0/Bin+Headers.zip
unzip Bin+Headers.zip -d ../../packages/react-native-webgpu
```

<details>
<summary>Manual instructions</summary>

- Download `Bin+Headers.zip` from the [release page](https://github.com/seanhenry/react-native-webgpu/releases/tag/v0.0.0).
- Unzip and move the `bin` and `include` folders to `../../packages/react-native-webgpu`
</details>

5. Install pods (iOS only)

```bash
yarn pod:install:newArch
```

6. Build and run

```bash
yarn ios
yarn android
```
