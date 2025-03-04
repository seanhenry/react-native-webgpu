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
import {examplesCallback} from '../../../utils/examplesCallback';

export const Cornell = () => {
  const {gui, Controls} = useControls();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const surfaceCapabilities = context.surfaceCapabilities(adapter!);
    const needsWorkaround =
      (surfaceCapabilities.usages & GPUTextureUsage.STORAGE_BINDING) === 0;
    if (needsWorkaround) {
      console.warn(
        ">> STORAGE_BINDING isn't supported on surface texture, using workaround",
      );
    }
    // Issue 1: xxx-srgb cannot have STORAGE_BINGING so don't use that
    const presentationFormat = navigator.gpu
      .getPreferredCanvasFormat()
      .replace(/-srgb$/, '') as GPUTextureFormat;
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

    let surfaceUsage =
      GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_DST;
    // Issue 2: If STORAGE_BINDING is not supported, don't request it
    if (!needsWorkaround) {
      surfaceUsage |= GPUTextureUsage.STORAGE_BINDING;
    }
    context.configure({
      device,
      format: presentationFormat,
      usage: surfaceUsage,
      alphaMode: 'auto',
    });

    // Issue 3: context.getCurrentTexture() STORAGE_BINDING not supported
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
        // Issue 4: Since canvasTexture is no longer our copy destination, we
        // copy it as a final step.
        commandEncoder.copyTextureToTexture(
          {texture: copyTexture},
          {texture: canvasTexture},
          canvasTexture,
        );
      }

      device.queue.submit([commandEncoder.finish()]);
      examplesCallback(device.queue);
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
