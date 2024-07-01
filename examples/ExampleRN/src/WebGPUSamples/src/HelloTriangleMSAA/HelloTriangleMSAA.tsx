import { WebGpuView } from 'react-native-webgpu';
import triangleVertWGSL from '../../shaders/triangle.vert.wgsl';
import redFragWGSL from '../../shaders/red.frag.wgsl';
import { CenterSquare } from '../../../Components/CenterSquare';
import React from 'react';

export function HelloTriangleMSAA() {
  const onInit = async ({ identifier }: { identifier: string }) => {

    const { navigator, getContext } = global.webGPU;

    const context = getContext({ identifier });
    const adapter = await navigator.gpu.requestAdapter({context});
    const device = await adapter!.requestDevice();

    const { formats, alphaModes } = context.surfaceCapabilities;
    const { width, height } = context;
    const presentationFormat = formats[0]!

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: alphaModes[0],
    });

    const sampleCount = 4;

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
        entryPoint: 'main',
        targets: [
          {
            format: presentationFormat,
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
      },
      multisample: {
        count: sampleCount,
      },
    });

    const texture = device.createTexture({
      size: [width, height],
      sampleCount,
      format: presentationFormat,
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
    });
    const view = texture.createView();

    function frame() {
      const commandEncoder = device.createCommandEncoder();
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return
      }

      const renderPassDescriptor: GPURenderPassDescriptor = {
        colorAttachments: [
          {
            view,
            resolveTarget: framebuffer.createView(),
            clearValue: [0, 0, 0, 1],
            loadOp: 'clear',
            storeOp: 'discard',
          },
        ],
      };

      const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
      passEncoder.setPipeline(pipeline);
      passEncoder.draw(3);
      passEncoder.end();

      device.queue.submit([commandEncoder.finish()]);
      context.presentSurface();

      framebuffer.destroy();
      requestAnimationFrame(frame);
    }

    requestAnimationFrame(frame);
  };
  return (
    <CenterSquare>
      <WebGpuView identifier="HelloTriangleMSAA" onInit={onInit} style={{ width: '100%', height: '100%' }} />
    </CenterSquare>
  );
}
