import {mat4, vec3} from 'wgpu-matrix';
import {OnCreateSurfacePayload} from 'react-native-webgpu';
import triangleWGSL from './triangle.wgsl';
import {examplesCallback} from '../../utils/examplesCallback';

export const runResizeCanvas = async ({
  context,
  navigator,
  requestAnimationFrame,
}: OnCreateSurfacePayload) => {
  const adapter = await navigator.gpu.requestAdapter();
  const device = await adapter!.requestDevice();

  const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

  context.configure({
    device,
    format: presentationFormat,
    alphaMode: 'auto',
  });

  const vertexBuffer = device.createBuffer({
    usage: GPUBufferUsage.VERTEX,
    size: 4 * 4 * 3,
    mappedAtCreation: true,
  });
  // prettier-ignore
  new Float32Array(vertexBuffer.getMappedRange()).set([
    0, 1, 0, 1,
    -1, -1, 0, 1,
    1, -1, 0, 1,
  ]);
  vertexBuffer.unmap();

  const pipeline = device.createRenderPipeline({
    layout: 'auto',
    vertex: {
      entryPoint: 'v_main',
      module: device.createShaderModule({
        code: triangleWGSL,
      }),
      buffers: [
        {
          stepMode: 'vertex',
          arrayStride: 4 * 4,
          attributes: [
            {
              format: 'float32x4',
              offset: 0,
              shaderLocation: 0,
            },
          ],
        },
      ],
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
  });
  const uniformsBuffer = device.createBuffer({
    usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    size: 4 * 4 * 4,
  });
  const bindGroup = device.createBindGroup({
    layout: pipeline.getBindGroupLayout(0),
    entries: [
      {
        binding: 0,
        resource: {buffer: uniformsBuffer},
      },
    ],
  });

  const previousSize = {width: context.width, height: context.height};
  function frame() {
    if (
      context.width !== previousSize.width ||
      context.height !== previousSize.height
    ) {
      context.configure({
        device,
        format: presentationFormat,
        alphaMode: 'auto',
      });
    }
    previousSize.width = context.width;
    previousSize.height = context.height;

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

    const projectionMat = mat4.ortho(0, context.width, 0, context.height, 0, 1);
    const viewMat = mat4.lookAt(
      vec3.create(0, 0, 1),
      vec3.create(0, 0, 0),
      vec3.create(0, 1, 0),
    );
    const width = context.width / 4;
    const mat = mat4.scale(
      mat4.translate(
        mat4.multiply(projectionMat, viewMat),
        vec3.create(context.width / 2, context.height / 2, 0),
      ),
      vec3.create(width, width, 1),
    );
    device.queue.writeBuffer(uniformsBuffer, 0, mat.buffer);

    const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
    passEncoder.setPipeline(pipeline);
    passEncoder.setBindGroup(0, bindGroup);
    passEncoder.setVertexBuffer(0, vertexBuffer);
    passEncoder.draw(3);
    passEncoder.end();

    device.queue.submit([commandEncoder.finish()]);
    examplesCallback(device.queue);
    context.presentSurface();
    requestAnimationFrame(frame);
  }
  requestAnimationFrame(frame);
};
