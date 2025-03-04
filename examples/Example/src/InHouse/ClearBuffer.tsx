import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {useControls} from '../Components/controls/react/useControls';
import {PixelRatio} from 'react-native';
import {examplesCallback} from '../utils/examplesCallback';

export const ClearBuffer = () => {
  const {gui, Controls} = useControls();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();
    const preferredFormat = navigator.gpu.getPreferredCanvasFormat();
    context.configure({
      device,
      format: preferredFormat,
      usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_DST,
    });

    const params = {
      rowStartOffset: 0,
      rowEndOffset: 10,
    };
    gui.add(params, 'rowStartOffset', 0, context.height / 2);
    gui.add(params, 'rowEndOffset', 0, context.height / 2);
    gui.draw();

    const bytesPerRow = context.width * 4;
    const numPixels = context.width * context.height;
    const green = new Array(numPixels)
      .fill(null)
      .flatMap(() => [0, 255, 0, 255]);

    const buffer = device.createBuffer({
      size: bytesPerRow * context.height,
      usage: GPUBufferUsage.COPY_SRC | GPUBufferUsage.COPY_DST,
    });

    const frame = () => {
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }

      device.queue.writeBuffer(buffer, 0, new Uint8Array(green).buffer);

      const encoder = device.createCommandEncoder();

      encoder.clearBuffer(
        buffer,
        params.rowStartOffset * bytesPerRow,
        params.rowEndOffset * bytesPerRow,
      );
      encoder.copyBufferToTexture(
        {buffer, bytesPerRow},
        {texture: framebuffer},
        context,
      );

      device.queue.submit([encoder.finish()]);
      examplesCallback(device.queue);
      context.presentSurface();
      requestAnimationFrame(frame);
    };

    requestAnimationFrame(frame);
  };

  // copyBufferToTexture requires buffers dimensions are multiples of 256
  const size = 256 / PixelRatio.get();
  return (
    <>
      <WebGpuView
        onCreateSurface={onCreateSurface}
        style={{
          width: size,
          height: size,
        }}
      />
      <Controls />
    </>
  );
};
