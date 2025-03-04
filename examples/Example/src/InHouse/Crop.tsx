import {Square} from '../Components/Square';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {useControls} from '../Components/controls/react/useControls';
import {Controller} from '../Components/controls/dat.gui/Controller';
import videoQuad from '../utils/shaders/videoQuad.wgsl';
import {examplesCallback} from '../utils/examplesCallback';

export const Crop = () => {
  const {gui, Controls} = useControls();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    createImageBitmap,
    requestAnimationFrame,
  }) => {
    let needsCopy = true;
    const image = await createImageBitmap(
      require('../assets/images/cropImage.png'),
    );
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();
    const preferredFormat = navigator.gpu.getPreferredCanvasFormat();
    context.configure({
      device,
      format: preferredFormat,
    });

    const sampler = device.createSampler();
    const module = device.createShaderModule({
      code: videoQuad,
    });
    const pipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        entryPoint: 'v_main',
        module: module,
      },
      fragment: {
        entryPoint: 'f_main',
        module: module,
        targets: [{format: preferredFormat}],
      },
    });

    // Controls
    const params = {
      flipY: false,
      top: 0,
      right: 0,
      bottom: 0,
      left: 0,
      crop: '',
    };
    const setNeedsCopy = () => {
      needsCopy = true;
    };
    const flipYController = gui.add(params, 'flipY').onChange(setNeedsCopy);
    const controllers: Controller<any, any, number>[] = [
      gui.add(params, 'top', 0, image.width - 1).onChange(setNeedsCopy),
      gui.add(params, 'right', 0, image.height - 1).onChange(setNeedsCopy),
      gui.add(params, 'bottom', 0, image.width - 1).onChange(setNeedsCopy),
      gui.add(params, 'left', 0, image.height - 1).onChange(setNeedsCopy),
    ];
    const cropController = gui.add(params, 'crop');
    cropController.disabled = true;
    const controls = {
      reset() {
        flipYController.setValue(false);
        controllers.forEach(controller => controller.setValue(0));
      },
      cropToContents() {
        controllers.forEach(controller => controller.setValue(100));
      },
    };
    gui.add(controls, 'reset');
    gui.add(controls, 'cropToContents').name('crop to contents');
    gui.draw();

    let imageTexture: GPUTexture;
    let bindGroup: GPUBindGroup;
    function frame() {
      const texture = context.getCurrentTexture();
      if (!texture) {
        requestAnimationFrame(frame);
        return;
      }

      const {top, right, bottom, left, flipY} = params;
      const x = left;
      const y = bottom;
      const width = Math.max(1, image.width - right - left);
      const height = Math.max(1, image.height - top - bottom);
      cropController.setValue(`[${x} ${y} ${width} ${height}]`);
      if (width !== imageTexture?.width || height !== imageTexture?.height) {
        imageTexture = device.createTexture({
          format: 'rgba8unorm-srgb',
          size: [width, height],
          usage:
            GPUTextureUsage.RENDER_ATTACHMENT |
            GPUTextureUsage.COPY_DST |
            GPUTextureUsage.TEXTURE_BINDING,
        });
        bindGroup = device.createBindGroup({
          layout: pipeline.getBindGroupLayout(0),
          entries: [
            {
              binding: 0,
              resource: sampler,
            },
            {
              binding: 1,
              resource: imageTexture.createView(),
            },
          ],
        });
      }

      if (needsCopy) {
        needsCopy = false;
        device.queue.copyExternalImageToTexture(
          {
            flipY,
            origin: [x, y],
            source: image,
          },
          {
            texture: imageTexture,
          },
          [width, height],
        );
      }

      const encoder = device.createCommandEncoder();
      const render = encoder.beginRenderPass({
        colorAttachments: [
          {
            view: texture.createView(),
            loadOp: 'clear',
            storeOp: 'store',
            clearValue: [0, 0, 0, 1],
          },
        ],
      });
      render.setPipeline(pipeline);
      render.setBindGroup(0, bindGroup);
      render.draw(6);
      render.end();
      device.queue.submit([encoder.finish()]);
      examplesCallback(device.queue);
      context.presentSurface();

      requestAnimationFrame(frame);
    }

    requestAnimationFrame(frame);
  };
  return (
    <>
      <Square>
        <WebGpuView onCreateSurface={onCreateSurface} />
      </Square>
      <Controls />
    </>
  );
};
