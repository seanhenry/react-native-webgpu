# Compatibility notes

It's not possible to use `three` out of the box as it has some dependencies on web apis.

This file outlines the steps taken to make `three` compatible.

## v0.166.1

### (0, _$$_REQUIRE(_dependencyMap[5(...)tribute)('faceIndex')).normalize is not a function ... 

`three/examples/jsm/renderers/common/extras/PMREMGenerator.js:79-80` tries to access functions before they have been added.

Solution: add `require('three/examples/jsm/nodes/math/MathNode.js')` before importing `three`.  
See: `src/index.js`

### Nodes.js:174 Export should first be transformed by @babel/plugin-transform-export-namespace-from

`export * as PMREMUtils ...` needs to be transformed by babel.

Solution: add `@babel/plugin-transform-export-namespace-from` plugin to `babel.config.js`.  
See: `babel/index.js`.

### Compiling JS failed: 217753:26:unexpected token after assignment expression Buffer size 12720419 starts with: 766172205f5f42554e444c455f535441 ...

`WebGPU.js` uses `await` at the root level to determine if webgpu is available or not.

Solution: Copy file and use `metro.config.js` to import our version.  
See: `src/examples/jsm/capabilities/WebGPU.js` and `metro/index.js`

### Unable to resolve module three/addons/renderers/webgpu/WebGPURenderer.js ...

`three` uses `exports` in `package.json` to point `three/addons/*` to `three/examples/jsm/*`. Metro has [experimental support](https://metrobundler.dev/docs/package-exports/) for package exports but we can work around it.

Solution: 
- Use `metro.config.js` to point `three/addons/*` to the correct directory.
- Add `unstable_enablePackageExports: true` to `metro.config.js`
See `metro/index.js`

### Texture format on Android is wrong

Android's surface view only supports rgb formats whereas `three` is hardcoded to use `bgra8unorm`.

Solution: set `GPUTextureFormat.BGRA8Unorm` to `navigator.gpu.getPreferredCanvasFormat()`.
See: `src/ThreeWebGpuView.js`  
TODO: can we solve this in a better way so `GPUTextureFormat.BGRA8Unorm` can still be used elsewhere?

### WebGPURenderer

`WebGPURenderer` accesses web apis unless we provide the appropriate params: `context`, `canvas` and `device`.

Solution: 
- Provide the context
- Create a device and pass it in
- Use a `Proxy` to act as the canvas and return `width` and `height`
See: `rendererParams` in `src/ThreeWebGpuView.js`

### setAnimationLoop

`Renderer.setAnimationLoop` will not stop when the webgpu view is unmounted so this must be done manually.

Solution: `onCreateSurface` provides a callback to perform a teardown. Call `renderer.setAnimationLoop(null)` in the teardown.

### TextDecoder

`TextDecoder` is used by `GLTFLoader` (and others)

Solution: Add `fast-text-encoding` polyfill  
See: `src/index.js`

### TypeError: Invalid attempt to destructure non-iterable instance

`NodeBuilder.js:1006`: `shaderNode.isArrayInput` is checking if the function receives an array using `function.toString()`.

Solution: Add patch to make array with node properties so it will work for both.  
See: `src/nodeBuilderPatch.js`

### Shader validation error ... `i32( 20.0 )`

`ConstNode` is of type `float` but when is converted to `int` it produces `i32( 20.0 )` which does not compile.

Solution: Add patch to produce `20` instead of `i32( 20.0 )`  
See: `src/constNodePatch.js`
