// https://webgpu.github.io/webgpu-samples/?sample=helloTriangle
import {WebGpuView, type WebGpuViewProps} from 'react-native-webgpu';
import triangleWGSL from './cw-triangle.wgsl';
import {Square} from '../../Components/Square';
import {examplesCallback} from '../../utils/examplesCallback';

export function CWTriangle() {
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'auto',
    });

    const pipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        entryPoint: 'v_main',
        module: device.createShaderModule({
          code: triangleWGSL,
        }),
      },
      fragment: {
        entryPoint: 'f_main',
        module: device.createShaderModule({
          code: triangleWGSL,
        }),
        targets: [
          {
            format: presentationFormat,
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
        frontFace: 'cw', // change front to be clockwise
        cullMode: 'back', // culling back to prove it works
      },
    });

    function frame() {
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
      };

      const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
      passEncoder.setPipeline(pipeline);
      passEncoder.draw(3);
      passEncoder.end();

      device.queue.submit([commandEncoder.finish()]);
      examplesCallback(device.queue);
      context.presentSurface();
      requestAnimationFrame(frame);
    }
    requestAnimationFrame(frame);
  };

  return (
    <Square>
      <WebGpuView onCreateSurface={onCreateSurface} />
    </Square>
  );
}
