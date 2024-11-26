# react-native-webgpu

A [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) port for react native to provide direct access to Metal and Vulkan for iOS and Android via the WebGPU api.

## Contents

- [Running the examples](#running-the-examples)
- [Getting started](#getting-started)
- [Converting a WebGPU sample](#converting-a-webgpu-sample)
- [Resizing WebGpuView](#resizing-webgpuview)
- [Production usage](#production-use)
- [Supported react-native versions](#supported-react-native-versions)

## Running the examples

The best way to learn WebGPU is to check out the examples. You can find instructions to run the examples [here](../../examples/Example).

## Getting started

- Install packages

```shell
yarn add react-native-webgpu
```

- Install pods

```shell
cd ios
pod install
cd ..
```

- Import `react-native-webgpu` in your root `index.js` file so it can install on app startup

```javascript
// src/index.js
import 'react-native-webgpu';
```

- Add to `metro.config.js` to support `.wgsl` files

```javascript
// metro.config.js
const defaultConfig = getDefaultConfig(__dirname);
const webGpuConfig = require('react-native-webgpu/metro');
const config = {
  resolver: {
    sourceExts: [
      ...defaultConfig.resolver.sourceExts,
      ...webGpuConfig.resolver.sourceExts,
    ],
  },
  transformer: {
    babelTransformerPath: require.resolve('react-native-webgpu/wgsl-babel-transformer'),
  },
};
```

- TypeScript only, add global types to `tsconfig.json`

```json
{
  "include": [
    "node_modules/react-native-webgpu/lib/typescript/react-native-webgpu.d.ts"
  ]
}
```

### Android emulator ⚠️

The Android emulator doesn't support hardware acceleration for Vulkan so it will crash when attempting to use the Vulkan backend. To work around it, you can set the `backends` prop to GL when using the emulator.

```typescript jsx
<WebGpuView backends={Platform.OS === android && isEmulator ? Backends.GL : Backends.All} />
```

Or you can set the default `backends` prop globally.

```typescript
defaultBackends.current =
  Platform.OS === 'android' && isEmulator ? Backends.GL : Backends.All;
```

Please note, it's not safe to assume that the GL backend will be identical to Vulkan.
Be sure to test fully on all backends used in production.

## Converting a WebGPU sample

There are a few small changes you will need to make to get your project working. Below is a simple example taken from [WebGPU Samples](https://webgpu.github.io/webgpu-samples/?sample=helloTriangle). It has `TODO:`s marking the places we need to change.

<details>
<summary>Expand to view the code</summary>

```typescript
import triangleVertWGSL from '../../shaders/triangle.vert.wgsl';
import redFragWGSL from '../../shaders/red.frag.wgsl';
import { quitIfWebGPUNotAvailable } from '../util';

const canvas = document.querySelector('canvas') as HTMLCanvasElement; // TODO: Remove web api
const adapter = await navigator.gpu?.requestAdapter(); // TODO: Use the navigator from `react-native-webgpu` instead of `global`
const device = await adapter?.requestDevice();
quitIfWebGPUNotAvailable(adapter, device); // TODO: Remove since web gpu is always supported 🎉

const context = canvas.getContext('webgpu') as GPUCanvasContext; // TODO: Use the context from `react-native-webgpu`

const devicePixelRatio = window.devicePixelRatio; // TODO: Remove sizing as we use React to layout our views
canvas.width = canvas.clientWidth * devicePixelRatio;
canvas.height = canvas.clientHeight * devicePixelRatio;
const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

context.configure({
  device,
  format: presentationFormat,
  alphaMode: 'premultiplied',
});

const pipeline = device.createRenderPipeline({
  layout: 'auto',
  vertex: {
    // TODO: `entryPoint` must be specified in `react-native-webgpu`
    module: device.createShaderModule({
      code: triangleVertWGSL,
    }),
  },
  fragment: {
    // TODO: `entryPoint` must be specified in `react-native-webgpu`
    module: device.createShaderModule({
      code: redFragWGSL,
    }),
    targets: [
      {
        format: presentationFormat,
      },
    ],
  },
  primitive: {
    topology: 'triangle-list',
  },
});

function frame() {
  const commandEncoder = device.createCommandEncoder();
  const textureView = context.getCurrentTexture().createView();

  const renderPassDescriptor: GPURenderPassDescriptor = {
    colorAttachments: [
      {
        view: textureView,
        clearValue: [0, 0, 0, 1],
        loadOp: 'clear',
        storeOp: 'store',
      },
    ],
  };

  const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
  passEncoder.setPipeline(pipeline);
  passEncoder.draw(3);
  passEncoder.end();

  device.queue.submit([commandEncoder.finish()]);
  // TODO: We need to tell the surface to present itself onscreen
  requestAnimationFrame(frame);
}
// TODO: Use `requestAnimationFrame` from `react-native-webgpu` so it is called in sync with the screen refresh rate, and automatically cancels on unmount
requestAnimationFrame(frame);
```

</details>

Here is a working (TypeScript) example. It has `FIXED:` comments to show where the changes were made.

<details>
<summary>Expand to view the code</summary>

```typescript jsx
import React from 'react';
import { WebGpuView, type WebGpuViewProps } from 'react-native-webgpu';
import triangleVertWGSL from '../../shaders/triangle.vert.wgsl';
import redFragWGSL from '../../shaders/red.frag.wgsl';

export function HelloTriangle() {
  // FIXED: get context, navigator and requestAnimationFrame from `react-native-webgpu` callback
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({context, navigator, requestAnimationFrame}) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: "premultiplied",
    });

    const pipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        // FIXED: The shader function in `triangleVertWGSL` is called `main` so that's our entry point
        entryPoint: 'main',
        module: device.createShaderModule({
          code: triangleVertWGSL,
        }),
      },
      fragment: {
        // FIXED: The shader function in `redFragWGSL` is also called `main` so that's our entry point
        entryPoint: 'main',
        module: device.createShaderModule({
          code: redFragWGSL,
        }),
        targets: [
          {
            format: presentationFormat,
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
      },
    });

    function frame() {
      // FIXED: `getCurrentTexture()` can return `null` in `react-native-webgpu`
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }

      const commandEncoder = device.createCommandEncoder();
      const textureView = framebuffer.createView();

      const renderPassDescriptor: GPURenderPassDescriptor = {
        colorAttachments: [
          {
            view: textureView,
            clearValue: [0, 0, 0, 1],
            loadOp: 'clear',
            storeOp: 'store',
          },
        ],
      }

      const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
      passEncoder.setPipeline(pipeline);
      passEncoder.draw(3);
      passEncoder.end();

      device.queue.submit([commandEncoder.finish()]);
      // FIXED: Add context.presentSurface() to display the surface
      context.presentSurface();
      requestAnimationFrame(frame);
    }
    requestAnimationFrame(frame);
  };

  return <WebGpuView onCreateSurface={onCreateSurface} style={{flex: 1}} />;
}
```

</details>

## Resizing WebGpuView

If you expect `WebGpuView` to change size, you need to call `context.configure()` whenever the size changes.

<details>
<summary>Expand to view the code</summary>

```typescript
let previousWidth = context.width;
let previousHeight = context.height;
// ...

function frame() {
  if (context.width !== previousWidth || context.height !== previousHeight) {
    context.configure({device, format});
  }
  previousWidth = context.width;
  previousHeight = context.height;
  
  const framebuffer = context.getCurrentTexture(); // Now returns updated texture
  // ...
}
```

</details>

### Animating the size

If you want to smoothly change the size of `WebGpuView`, set the `pollSize` prop to `true`. This only affects iOS and
polls the surface size every frame to ensure it is correct. Setting this to `true` on Android has no effect because 
animations are supported without polling the size.

```typescript jsx
<WebGpuView onCreateSurface={onCreateSurface} pollSize />
```

## Production use

Like any third party code you introduce into your app, ensure that you thoroughly test on your supported platforms.

### Memory

Running loop-based, resource-heavy code in JavaScript environments can be challenging. The library is profiled for memory
usage, but you will need to test your app to make sure you're not accidentally introducing memory leaks.

Xcode Instruments and Android Studio Profiler are strongly recommended for profiling your app before releasing it to production.

## Supported react-native versions

The library is built and tested against 0.75 and 0.76. Other versions may work but are not supported.

| react-native-webgpu | react-native | Hermes | JSC | New architecture | Old architecture |
|---------------------|--------------|--------|-----|------------------|------------------|
| 0.1.0               | 0.75-0.76    | ✅      | ❌   | ✅                | ✅                |

