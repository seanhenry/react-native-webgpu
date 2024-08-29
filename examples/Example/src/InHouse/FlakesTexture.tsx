import {Square} from '../Components/Square';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../Components/globalStyles';
import quadWgsl from '../utils/shaders/quad.wgsl';
import {FlakesTextureGenerator} from '../utils/FlakesTextureGenerator';

/**
 * Three provides FlakesTexture class for generating a rough texture using canvas 2d api.
 * Since we can't use that, this is a WebGPU replacement. See `Three/Clearcoat` example.
 */
export const FlakesTexture = () => {
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const presentationFormat = context
      .surfaceCapabilities(adapter!)
      .formats.find(format => !format.endsWith('-srgb'));

    if (!presentationFormat) {
      console.error('Could not find non-srgb presentation format');
      return;
    }

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'opaque',
    });

    const generator = new FlakesTextureGenerator(device, context.width);
    generator.generate();
    const texture = generator.texture;
    if (!texture) {
      console.error('Failed to generate texture');
      return;
    }

    const module = device.createShaderModule({
      code: quadWgsl,
    });
    const renderPipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        entryPoint: 'v_main',
        module,
      },
      fragment: {
        entryPoint: 'f_main',
        module,
        targets: [{format: presentationFormat}],
      },
    });
    const sampler = device.createSampler();
    const bindGroupLayout = renderPipeline.getBindGroupLayout(0);
    const bindGroup = device.createBindGroup({
      layout: bindGroupLayout,
      entries: [
        {binding: 0, resource: sampler},
        {binding: 1, resource: texture.createView()},
      ],
    });

    const framebuffer = context.getCurrentTexture();
    if (!framebuffer) {
      return;
    }

    const encoder = device.createCommandEncoder();
    const renderPass = encoder.beginRenderPass({
      colorAttachments: [
        {
          view: framebuffer.createView(),
          loadOp: 'clear',
          storeOp: 'store',
          clearValue: [1, 0, 0, 1],
        },
      ],
    });
    renderPass.setPipeline(renderPipeline);
    renderPass.setBindGroup(0, bindGroup);
    renderPass.draw(6);
    renderPass.end();
    device.queue.submit([encoder.finish()]);
    context.presentSurface();
  };

  return (
    <Square>
      <WebGpuView onCreateSurface={onCreateSurface} style={globalStyles.fill} />
    </Square>
  );
};
