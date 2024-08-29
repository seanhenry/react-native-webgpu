import Scene from './scene';
import Common from './common';
import Radiosity from './radiosity';
import Rasterizer from './rasterizer';
import Tonemapper from './tonemapper';
import Raytracer from './raytracer';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {Square} from '../../../Components/Square';
import {globalStyles} from '../../../Components/globalStyles';
import {useControls} from '../../../Components/controls/react/useControls';

export const Cornell = () => {
  const {gui, Controls} = useControls();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const needsWorkaround = true;
    // const presentationFormat = navigator.gpu.getPreferredCanvasFormat();
    // Issue 1: xxx-srgb cannot have STORAGE_BINGING so don't use that
    const presentationFormat = needsWorkaround
      ? context
          .surfaceCapabilities(adapter!)
          .formats.find(format => !format.endsWith('-srgb'))!
      : navigator.gpu.getPreferredCanvasFormat();
    const requiredFeatures: GPUFeatureName[] =
      presentationFormat === 'bgra8unorm' ? ['bgra8unorm-storage'] : [];

    for (const feature of requiredFeatures) {
      if (!adapter!.features.has(feature)) {
        throw new Error(
          `sample requires ${feature}, but is not supported by the adapter`,
        );
      }
    }
    const device = await adapter!.requestDevice({requiredFeatures});

    const params: {
      renderer: 'rasterizer' | 'raytracer';
      rotateCamera: boolean;
    } = {
      renderer: 'rasterizer',
      rotateCamera: true,
    };

    gui.add(params, 'renderer', ['rasterizer', 'raytracer']);
    gui.add(params, 'rotateCamera' /*, true*/);
    gui.draw();

    context.configure({
      device,
      format: presentationFormat,
      usage:
        GPUTextureUsage.RENDER_ATTACHMENT |
        GPUTextureUsage.STORAGE_BINDING |
        GPUTextureUsage.COPY_DST,
      alphaMode: 'premultiplied',
    });

    // Issue 2: context.getCurrentTexture() STORAGE_BINDING not supported
    // so we create our own texture instead.
    let copyTexture: GPUTexture;
    if (needsWorkaround) {
      copyTexture = device.createTexture({
        label: 'framebuffer',
        size: [context.width, context.height],
        format: presentationFormat,
        usage:
          GPUTextureUsage.RENDER_ATTACHMENT |
          GPUTextureUsage.STORAGE_BINDING |
          GPUTextureUsage.COPY_SRC,
      });
    }

    const framebuffer = device.createTexture({
      label: 'framebuffer',
      size: [context.width, context.height],
      format: 'rgba16float',
      usage:
        GPUTextureUsage.RENDER_ATTACHMENT |
        GPUTextureUsage.STORAGE_BINDING |
        GPUTextureUsage.TEXTURE_BINDING,
    });

    const scene = new Scene(device);
    const common = new Common(device, scene.quadBuffer);
    const radiosity = new Radiosity(device, common, scene);
    const rasterizer = new Rasterizer(
      device,
      common,
      scene,
      radiosity,
      framebuffer,
    );
    const raytracer = new Raytracer(device, common, radiosity, framebuffer);

    function frame() {
      const canvasTexture = context.getCurrentTexture();
      if (!canvasTexture) {
        requestAnimationFrame(frame);
        return;
      }
      const commandEncoder = device.createCommandEncoder();

      common.update({
        rotateCamera: params.rotateCamera,
        aspect: context.width / context.height,
      });
      radiosity.run(commandEncoder);

      switch (params.renderer) {
        case 'rasterizer': {
          rasterizer.run(commandEncoder);
          break;
        }
        case 'raytracer': {
          raytracer.run(commandEncoder);
          break;
        }
      }

      const tonemapper = new Tonemapper(
        device,
        common,
        framebuffer,
        needsWorkaround ? copyTexture : canvasTexture,
      );
      tonemapper.run(commandEncoder);

      if (needsWorkaround) {
        // Issue 3: Since canvasTexture is no longer our copy destination, we
        // copy is as a final step.
        commandEncoder.copyTextureToTexture(
          {texture: copyTexture},
          {texture: canvasTexture},
          canvasTexture,
        );
      }

      device.queue.submit([commandEncoder.finish()]);
      context.presentSurface();

      requestAnimationFrame(frame);
    }

    requestAnimationFrame(frame);
  };
  return (
    <>
      <Square>
        <WebGpuView
          onCreateSurface={onCreateSurface}
          style={globalStyles.fill}
        />
      </Square>
      <Controls />
    </>
  );
};
