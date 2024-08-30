import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../../../Components/globalStyles';
import {Square} from '../../../Components/Square';

import {mat4} from 'wgpu-matrix';
import volumeWGSL from './volume.wgsl';
import {useControls} from '../../../Components/controls/react/useControls';
import {WEB_GPU_SAMPLES_BASE_URL} from '../../../utils/constants';

export const VolumeRendering = () => {
  const {gui, Controls} = useControls();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
  }) => {
    // GUI parameters
    const params: {rotateCamera: boolean; near: number; far: number} = {
      rotateCamera: true,
      near: 2.0,
      far: 7.0,
    };

    gui.add(params, 'rotateCamera');
    gui.add(params, 'near', 2.0, 7.0);
    gui.add(params, 'far', 2.0, 7.0);
    gui.draw();

    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const sampleCount = 4;

    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'auto',
    });

    const pipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        entryPoint: 'vertex_main',
        module: device.createShaderModule({
          code: volumeWGSL,
        }),
      },
      fragment: {
        entryPoint: 'fragment_main',
        module: device.createShaderModule({
          code: volumeWGSL,
        }),
        targets: [
          {
            format: presentationFormat,
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
        cullMode: 'back',
      },
      multisample: {
        count: sampleCount,
      },
    });

    const texture = device.createTexture({
      size: [context.width, context.height],
      sampleCount,
      format: presentationFormat,
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
    });
    const view = texture.createView();

    const uniformBufferSize = 4 * 16; // 4x4 matrix
    const uniformBuffer = device.createBuffer({
      size: uniformBufferSize,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    // Fetch the image and upload it into a GPUTexture.
    let volumeTexture: GPUTexture;
    {
      const width = 180;
      const height = 216;
      const depth = 180;
      const format: GPUTextureFormat = 'r8unorm';
      const blockLength = 1;
      const bytesPerBlock = 1;
      const blocksWide = Math.ceil(width / blockLength);
      const blocksHigh = Math.ceil(height / blockLength);
      const bytesPerRow = blocksWide * bytesPerBlock;
      const response = await fetch(
        `${WEB_GPU_SAMPLES_BASE_URL}/assets/img/volume/t1_icbm_normal_1mm_pn0_rf0_180x216x180_uint8_1x1.bin-gz`,
      );
      const arrayBuffer = await response.arrayBuffer();
      const decompressed = reactNativeWebGPU.experimental.inflate(arrayBuffer);
      const byteArray = new Uint8Array(decompressed);

      volumeTexture = device.createTexture({
        dimension: '3d',
        size: [width, height, depth],
        format: format,
        usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST,
      });

      device.queue.writeTexture(
        {
          texture: volumeTexture,
        },
        byteArray,
        {bytesPerRow: bytesPerRow, rowsPerImage: blocksHigh},
        [width, height, depth],
      );
    }

    // Create a sampler with linear filtering for smooth interpolation.
    const sampler = device.createSampler({
      magFilter: 'linear',
      minFilter: 'linear',
      mipmapFilter: 'linear',
      maxAnisotropy: 16,
    });

    const uniformBindGroup = device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [
        {
          binding: 0,
          resource: {
            buffer: uniformBuffer,
          },
        },
        {
          binding: 1,
          resource: sampler,
        },
        {
          binding: 2,
          resource: volumeTexture.createView(),
        },
      ],
    });

    const renderPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          view: undefined as unknown as GPUTextureView, // Assigned later

          clearValue: [0.5, 0.5, 0.5, 1.0],
          loadOp: 'clear',
          storeOp: 'discard',
        },
      ],
    };

    let rotation = 0;

    function getInverseModelViewProjectionMatrix(deltaTime: number) {
      const viewMatrix = mat4.identity();
      mat4.translate(viewMatrix, [0, 0, -4], viewMatrix);
      if (params.rotateCamera) {
        rotation += deltaTime;
      }
      mat4.rotate(
        viewMatrix,
        [Math.sin(rotation), Math.cos(rotation), 0],
        1,
        viewMatrix,
      );

      const aspect = context.width / context.height;
      const projectionMatrix = mat4.perspective(
        (2 * Math.PI) / 5,
        aspect,
        params.near,
        params.far,
      );
      const modelViewProjectionMatrix = mat4.multiply(
        projectionMatrix,
        viewMatrix,
      );

      return mat4.invert(modelViewProjectionMatrix);
    }

    let lastFrameMS = Date.now();

    function frame() {
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }
      const now = Date.now();
      const deltaTime = (now - lastFrameMS) / 1000;
      lastFrameMS = now;

      const inverseModelViewProjection =
        getInverseModelViewProjectionMatrix(deltaTime);
      device.queue.writeBuffer(uniformBuffer, 0, inverseModelViewProjection);
      (
        renderPassDescriptor.colorAttachments as GPURenderPassColorAttachment[]
      )[0].view = view;
      (
        renderPassDescriptor.colorAttachments as GPURenderPassColorAttachment[]
      )[0].resolveTarget = framebuffer.createView();

      const commandEncoder = device.createCommandEncoder();
      const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
      passEncoder.setPipeline(pipeline);
      passEncoder.setBindGroup(0, uniformBindGroup);
      passEncoder.draw(3);
      passEncoder.end();
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
