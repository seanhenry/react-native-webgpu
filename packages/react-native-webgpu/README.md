# react-native-webgpu

Coming soon. A [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) port for react native which aims to provide direct access to Metal and Vulkan for iOS and Android, and support for libraries like [Three.js](https://threejs.org).

Interested in this project? Star it in GitHub to show your support.

Other projects:
- [react-native-webgpu-three](../react-native-webgpu-three)

## Contents

- [Running the examples](#running-the-examples)
- [Getting started](#getting-started)
- [Converting a WebGPU sample](#converting-a-webgpu-sample)

## Running the examples

You can find instructions to run the examples [here](../../examples/Example).

## Getting started

<details>
<summary>
⚠️ Please note that these instructions don't work since this package has not yet been published
</summary>

- Install packages

```shell
yarn add react-native-webgpu
# For importing .wgsl files:
yarn add -D babel-plugin-inline-import 
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
import 'react-native-webgpu'
```

- Add the plugin to `babel.config.js` to support `.wgsl` files

```javascript
// babel.config.js
modules.exports = {
  plugins: [
    ['babel-plugin-inline-import', { extensions: ['.wgsl'] }],
  ],
};
```

- TypeScript only, add type definition for `.wgsl` files

```typescript
// index.d.ts
declare module '*.wgsl' {
  var value: string
  export default value
}
```

</details>

## Converting a WebGPU sample

There are a few small changes you will need to make to get your project working. Below is a simple example taken from [WebGPU Samples](https://webgpu.github.io/webgpu-samples/?sample=helloTriangle). It has `TODO:`s marking the places we need to change.

<details>
<summary>Click to expand</summary>

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
<summary>Click to expand</summary>

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

