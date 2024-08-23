# react-native-webgpu-three

Coming soon. Aims to provide support to run [Three.js](https://threejs.org) natively, in React Native.

Interested in this project? Star it in GitHub to show your support.

Looking for more direct access to the GPU through a JavaScript api? Check out [react-native-webgpu](https://github.com/seanhenry/react-native-webgpu).

## Contents

- [Running the examples](#running-the-examples)
- [Getting started](#getting-started)
- [Converting a three project](#converting-a-three-project)

## Running the examples

You can find instructions to run the examples [here](../../examples/Example).

## Getting started

<details>
<summary>
⚠️ Please note that these instructions don't work since this package has not yet been published
</summary>

- Install packages

```shell
yarn add react-native-webgpu-three react-native-webgpu three
yarn add -D @babel/plugin-transform-export-namespace-from
# For TypeScript also:
yarn add -D @types/three
```

- Install pods

```shell
cd ios
pod install
cd ..
```

- Import `react-native-webgpu-three` before importing `three`. It might be best to import it in `index.js` to be sure.

```javascript
// src/index.js
import 'react-native-webgpu-three';
```

- Add the plugins to `babel.config.js`

```javascript
// babel.config.js
modules.exports = {
  plugins: [...require('react-native-webgpu-three/babel').plugins],
};
```

- Add `resolveRequest` and `unstable_enablePackageExports` to `metro.config.js`

```javascript
// metro.config.js
module.exports = {
  resolver: {
    resolveRequest: require('react-native-webgpu-three/metro').resolver
      .resolveRequest,
    unstable_enablePackageExports: true,
  },
};
```

</details>

## Converting a three project

There are a few small changes you will need to make to get your scene working. Below is a simple example taken from the `three` [GitHub page](https://github.com/mrdoob/three.js?tab=readme-ov-file#usage). It has `TODO:`s marking the places we need to change.

```javascript
import * as THREE from 'three';

const width = window.innerWidth,
  height = window.innerHeight; // TODO: Replace web api

// init

const camera = new THREE.PerspectiveCamera(70, width / height, 0.01, 10);
camera.position.z = 1;

const scene = new THREE.Scene();

const geometry = new THREE.BoxGeometry(0.2, 0.2, 0.2);
const material = new THREE.MeshNormalMaterial();

const mesh = new THREE.Mesh(geometry, material);
scene.add(mesh);

const renderer = new THREE.WebGLRenderer({antialias: true}); // TODO: Use WebGPURenderer
renderer.setSize(width, height);
renderer.setAnimationLoop(animate); // TODO: Cancel animation loop on unmount
document.body.appendChild(renderer.domElement); // TODO: Remove web api

// animation

function animate(time) {
  mesh.rotation.x = time / 2000;
  mesh.rotation.y = time / 1000;

  renderer.render(scene, camera);
  // TODO: We need to tell the surface to present itself onscreen
}
```

Here is a working (TypeScript) example. It has `FIXED:` comments to show where the changes were made.

```typescript jsx
import React from 'react';
import { ThreeWebGpuView, type ThreeWebGpuViewProps } from 'react-native-webgpu-three';
import * as THREE from 'three';
import WebGPURenderer from 'three/addons/renderers/webgpu/WebGPURenderer.js';

export const HelloThree = () => {
  const onCreateSurface: ThreeWebGpuViewProps['onCreateSurface'] = async ({ context, rendererParameters }) => {
    // FIXED: Use the context to get the width and height instead of window
    const {width, height} = context

    const camera = new THREE.PerspectiveCamera(70, width / height, 0.01, 10);
    camera.position.z = 1;

    const scene = new THREE.Scene();

    const geometry = new THREE.BoxGeometry(0.2, 0.2, 0.2);
    const material = new THREE.MeshNormalMaterial();

    const mesh = new THREE.Mesh(geometry, material);
    scene.add(mesh);

    // FIXED: Use WebGPURenderer
    const renderer = new WebGPURenderer({
      antialias: true,
      // FIXED: Pass correct configuration
      ...rendererParameters,
    });
    await renderer.init();
    // FIXED: Deleted web api: document.body.appendChild( renderer.domElement )

    // animation

    function animate(time: number) {
      mesh.rotation.x = time / 2000;
      mesh.rotation.y = time / 1000;

      renderer.render(scene, camera);
      // FIXED: Add context.presentSurface() to display the surface
      context.presentSurface()
    }

    renderer.setAnimationLoop(animate);
    // FIXED: Return function called on unmount to cancel the animation loop
    return () => {
      renderer.setAnimationLoop(null)
    }
  };
  return <ThreeWebGpuView onCreateSurface={onCreateSurface} style={{flex: 1}} />;
};
```
