// https://webgpu.github.io/webgpu-samples/?sample=helloTriangleMSAA
import { WebGpuView, type WebGpuViewProps } from 'react-native-webgpu';
import triangleVertWGSL from '../../shaders/triangle.vert.wgsl';
import redFragWGSL from '../../shaders/red.frag.wgsl';
import { CenterSquare } from '../../../Components/CenterSquare';
import React from 'react';
import { globalStyles } from '../../../Components/globalStyles';

export function HelloTriangleMSAA() {
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({ context, navigator, requestAnimationFrame }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const { width, height } = context;
    const presentationFormat = navigator.gpu.getPreferredCanvasFormat()

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: "premultiplied",
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
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }

      const commandEncoder = device.createCommandEncoder();

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
