// https://webgpu.github.io/webgpu-samples/?sample=texturedCube
import {mat4, vec3} from 'wgpu-matrix';

import {
  cubeVertexArray,
  cubeVertexSize,
  cubeUVOffset,
  cubePositionOffset,
  cubeVertexCount,
} from '../../meshes/cube';

import basicVertWGSL from '../../shaders/basic.vert.wgsl';
import sampleTextureMixColorWGSL from './sampleTextureMixColor.frag.wgsl';
import {Square} from '../../../Components/Square';
import {WebGpuView, type WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../../../Components/globalStyles';
import {examplesCallback} from '../../../utils/examplesCallback';

export const TexturedCube = () => {
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
    createImageBitmap,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const {width, height} = context;
    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'auto',
    });

    // Create a vertex buffer from the cube data.
    const verticesBuffer = device.createBuffer({
      size: cubeVertexArray.byteLength,
      usage: GPUBufferUsage.VERTEX,
      mappedAtCreation: true,
    });
    new Float32Array(verticesBuffer.getMappedRange()).set(cubeVertexArray);
    verticesBuffer.unmap();

    const pipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        module: device.createShaderModule({
          code: basicVertWGSL,
        }),
        entryPoint: 'main',
        buffers: [
          {
            arrayStride: cubeVertexSize,
            attributes: [
              {
                // position
                shaderLocation: 0,
                offset: cubePositionOffset,
                format: 'float32x4',
              },
              {
                // uv
                shaderLocation: 1,
                offset: cubeUVOffset,
                format: 'float32x2',
              },
            ],
          },
        ],
      },
      fragment: {
        module: device.createShaderModule({
          code: sampleTextureMixColorWGSL,
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

        // Backface culling since the cube is solid piece of geometry.
        // Faces pointing away from the camera will be occluded by faces
        // pointing toward the camera.
        cullMode: 'back',
      },

      // Enable depth testing so that the fragment closest to the camera
      // is rendered in front.
      depthStencil: {
        depthWriteEnabled: true,
        depthCompare: 'less',
        format: 'depth24plus',
      },
    });

    const depthTexture = device.createTexture({
      size: [width, height],
      format: 'depth24plus',
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
    });

    const uniformBufferSize = 4 * 16; // 4x4 matrix
    const uniformBuffer = device.createBuffer({
      size: uniformBufferSize,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    // Fetch the image and upload it into a GPUTexture.
    let cubeTexture: GPUTexture;
    {
      const response = require('../../assets/img/Di-3d.png');
      const imageBitmap = await createImageBitmap(response);

      cubeTexture = device.createTexture({
        size: [imageBitmap.width, imageBitmap.height, 1],
        format: 'rgba8unorm',
        usage:
          GPUTextureUsage.TEXTURE_BINDING |
          GPUTextureUsage.COPY_DST |
          GPUTextureUsage.RENDER_ATTACHMENT,
      });
      device.queue.copyExternalImageToTexture(
        {source: imageBitmap},
        {texture: cubeTexture},
        [imageBitmap.width, imageBitmap.height],
      );

      imageBitmap.close();
    }

    // Create a sampler with linear filtering for smooth interpolation.
    const sampler = device.createSampler({
      magFilter: 'linear',
      minFilter: 'linear',
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
          resource: cubeTexture.createView(),
        },
      ],
    });

    const renderPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          view: undefined as unknown as GPUTextureView, // Assigned later

          clearValue: [0.5, 0.5, 0.5, 1.0],
          loadOp: 'clear',
          storeOp: 'store',
        },
      ],
      depthStencilAttachment: {
        view: depthTexture.createView(),

        depthClearValue: 1.0,
        depthLoadOp: 'clear',
        depthStoreOp: 'store',
      },
    };

    const aspect = width / height;
    const projectionMatrix = mat4.perspective(
      (2 * Math.PI) / 5,
      aspect,
      1,
      100.0,
    );
    const modelViewProjectionMatrix = mat4.create();

    function getTransformationMatrix() {
      const viewMatrix = mat4.identity();
      mat4.translate(viewMatrix, vec3.fromValues(0, 0, -4), viewMatrix);
      const now = Date.now() / 1000;
      mat4.rotate(
        viewMatrix,
        vec3.fromValues(Math.sin(now), Math.cos(now), 0),
        1,
        viewMatrix,
      );

      mat4.multiply(projectionMatrix, viewMatrix, modelViewProjectionMatrix);

      return modelViewProjectionMatrix;
    }

    function frame() {
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }

      const transformationMatrix = getTransformationMatrix();
      device.queue.writeBuffer(
        uniformBuffer,
        0,
        transformationMatrix.buffer,
        transformationMatrix.byteOffset,
        transformationMatrix.byteLength,
      );

      (
        renderPassDescriptor.colorAttachments as GPURenderPassColorAttachment[]
      )[0]!.view = framebuffer.createView();

      const commandEncoder = device.createCommandEncoder();
      const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
      passEncoder.setPipeline(pipeline);
      passEncoder.setBindGroup(0, uniformBindGroup);
      passEncoder.setVertexBuffer(0, verticesBuffer);
      passEncoder.draw(cubeVertexCount);
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
      <WebGpuView onCreateSurface={onCreateSurface} style={globalStyles.fill} />
    </Square>
  );
};
