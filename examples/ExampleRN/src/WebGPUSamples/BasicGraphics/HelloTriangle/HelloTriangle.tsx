// https://webgpu.github.io/webgpu-samples/?sample=helloTriangle
import { WebGpuView, type WebGpuViewProps } from 'react-native-webgpu';
import triangleVertWGSL from '../../shaders/triangle.vert.wgsl';
import redFragWGSL from '../../shaders/red.frag.wgsl';
import React from 'react';
import { CenterSquare } from '../../../Components/CenterSquare';
import { globalStyles } from '../../../Components/globalStyles';

export function HelloTriangle() {

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
        module: device.createShaderModule({
          code: triangleVertWGSL,
        }),
        entryPoint: 'main',
      },
      fragment: {
        module: device.createShaderModule({
          code: redFragWGSL,
        }),
        targets: [
          {
            format: presentationFormat,
          },
        ],
        entryPoint: 'main',
      },
      primitive: {
        topology: 'triangle-list',
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

      const passEncoder = commandEncoder.beginRenderPass({
        colorAttachments: [
          {
            view: textureView,
            clearValue: [0, 0, 0, 1],
            loadOp: 'clear',
            storeOp: 'store',
          },
        ],
      });
      passEncoder.setPipeline(pipeline);
      passEncoder.draw(3);
      passEncoder.end();

      device.queue.submit([commandEncoder.finish()]);
      context.presentSurface(framebuffer);
      requestAnimationFrame(frame);
    }
    requestAnimationFrame(frame);
  };

  return (
    <CenterSquare>
      <WebGpuView onCreateSurface={onCreateSurface} style={globalStyles.fill} />
    </CenterSquare>
  );
}
