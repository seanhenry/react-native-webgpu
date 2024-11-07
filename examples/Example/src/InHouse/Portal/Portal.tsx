import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../../Components/globalStyles';
import {mat4, vec3} from 'wgpu-matrix';
import {WebIO, type Accessor} from '@gltf-transform/core';
import modelGltf from './portal.gltf';
import portalWgsl from './portal.wgsl';

export const Portal = () => {
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
    createImageBitmap,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const preferredFormat = navigator.gpu.getPreferredCanvasFormat();
    context.configure({
      device,
      format: preferredFormat,
    });

    let viewProjectionMat = mat4.identity();
    const uniformsBuffer = device.createBuffer({
      size: viewProjectionMat.byteLength,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    const module = device.createShaderModule({
      code: portalWgsl,
    });
    const sampler = device.createSampler({});
    const image = await createImageBitmap(require('./portal.jpg'));
    const texture = device.createTexture({
      size: image,
      format: 'rgba8unorm-srgb',
      usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST,
    });
    device.queue.copyExternalImageToTexture({source: image}, {texture}, image);

    const document = await new WebIO().readJSON({
      json: modelGltf,
      resources: {},
    });
    const meshes = document.getRoot().listMeshes();

    let portal: Portal;
    {
      const mesh = meshes.find(m => m.getName() === 'Portal')!;
      const primitive = mesh.listPrimitives()[0]!;
      const position = primitive.getAttribute('POSITION')!;
      const indices = primitive.getIndices()!;

      const positionBuffer = createVertexBuffer(device, position);
      const indexBuffer = createIndexBuffer(device, indices);

      const pipeline = createPortalRenderPipeline({
        device,
        format: preferredFormat,
        position,
        module,
        stencilFront: {
          compare: 'never', // Never render
          failOp: 'replace', // Replace stencil value with 1
        },
      });
      const bindGroup0 = createBindGroup0(device, pipeline, uniformsBuffer);

      portal = {positionBuffer, indexBuffer, pipeline, bindGroup0, indices};
    }

    const scene: Scene = [
      meshes.find(mesh => mesh.getName() === 'Back')!,
      meshes.find(mesh => mesh.getName() === 'Front')!,
    ].map(mesh => {
      const primitive = mesh.listPrimitives()[0]!;
      const position = primitive.getAttribute('POSITION')!;
      const uvs = primitive.getAttribute('TEXCOORD_0')!;
      const indices = primitive.getIndices()!;

      const positionBuffer = createVertexBuffer(device, position);
      const uvBuffer = createVertexBuffer(device, uvs);
      const indexBuffer = createIndexBuffer(device, indices);
      const pipeline = createRenderPipeline({
        format: preferredFormat,
        uvs,
        position,
        device,
        module,
        stencilFront: {
          compare: 'not-equal', // Only render when stencil value is 0
          depthFailOp: 'keep',
          failOp: 'keep',
          passOp: 'keep',
        },
      });
      const bindGroup0 = createBindGroup0(device, pipeline, uniformsBuffer);
      const bindGroup1 = createBindGroup1(device, pipeline, sampler, texture);

      return {
        positionBuffer,
        uvBuffer,
        indexBuffer,
        pipeline,
        bindGroup0,
        bindGroup1,
        indices,
      };
    });

    const portalScene: Scene = [
      meshes.find(mesh => mesh.getName() === 'Mars')!,
      meshes.find(mesh => mesh.getName() === 'Space')!,
    ].map(mesh => {
      const primitive = mesh.listPrimitives()[0]!;
      const position = primitive.getAttribute('POSITION')!;
      const uvs = primitive.getAttribute('TEXCOORD_0')!;
      const indices = primitive.getIndices()!;

      const positionBuffer = createVertexBuffer(device, position);
      const uvBuffer = createVertexBuffer(device, uvs);
      const indexBuffer = createIndexBuffer(device, indices);
      const pipeline = createRenderPipeline({
        format: preferredFormat,
        uvs,
        position,
        device,
        module,
        stencilFront: {
          compare: 'equal', // Only render when stencil value is 1
          depthFailOp: 'keep',
          failOp: 'keep',
          passOp: 'keep',
        },
      });
      const bindGroup0 = createBindGroup0(device, pipeline, uniformsBuffer);
      const bindGroup1 = createBindGroup1(device, pipeline, sampler, texture);

      return {
        positionBuffer,
        indexBuffer,
        pipeline,
        bindGroup0,
        bindGroup1,
        indices,
        uvBuffer,
      };
    });

    // Depth and stencil texture
    const depthTexture = device.createTexture({
      size: context,
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
      format: 'depth24plus-stencil8',
    });
    const depthStencilAttachment: GPURenderPassDepthStencilAttachment = {
      view: depthTexture.createView(),
      depthLoadOp: 'load',
      depthStoreOp: 'store',
      stencilLoadOp: 'load',
      stencilStoreOp: 'store',
    };

    // Camera transformations
    const projectionMat = mat4.perspective(
      (2 * Math.PI) / 5,
      context.width / context.height,
      1,
      100,
    );
    const viewMat = mat4.lookAt(
      vec3.fromValues(0, 0, -5),
      vec3.fromValues(0, 0, 0),
      vec3.fromValues(0, 1, 0),
    );
    const sceneModelMat = mat4.identity();

    // @ts-expect-error missing type for performance
    let time = performance.now();

    const frame = (t: number) => {
      const dt = t - time;
      time = t;
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }
      mat4.rotateY(sceneModelMat, dt * 0.0008, sceneModelMat);

      mat4.multiply(projectionMat, viewMat, viewProjectionMat);
      mat4.multiply(viewProjectionMat, sceneModelMat, viewProjectionMat);
      device.queue.writeBuffer(
        uniformsBuffer,
        0,
        viewProjectionMat.buffer,
        viewProjectionMat.byteOffset,
        viewProjectionMat.byteLength,
      );

      const encoder = device.createCommandEncoder();

      // Clear depth/stencil
      const clearPass = encoder.beginRenderPass({
        colorAttachments: [
          {
            view: framebuffer.createView(),
            loadOp: 'clear',
            storeOp: 'store',
            clearValue: [1, 1, 1, 1],
          },
        ],
        depthStencilAttachment: {
          view: depthTexture.createView(),
          depthLoadOp: 'clear',
          depthStoreOp: 'store',
          stencilLoadOp: 'clear',
          stencilStoreOp: 'store',
          depthClearValue: 1.0,
        },
      });
      clearPass.end();

      device.queue.submit([encoder.finish()]);

      // Render portal (make stencil)
      renderPortal({
        device,
        portal,
        texture: framebuffer,
        depthStencilAttachment,
      });

      // Render scene (only render outside portal window)
      renderScene({
        device,
        scene,
        texture: framebuffer,
        depthStencilAttachment,
      });

      // Render portal scene (only render inside portal window)
      renderScene({
        device,
        scene: portalScene,
        texture: framebuffer,
        depthStencilAttachment,
      });

      context.presentSurface();
      requestAnimationFrame(frame);
    };
    requestAnimationFrame(frame);
  };
  return (
    <WebGpuView onCreateSurface={onCreateSurface} style={globalStyles.fill} />
  );
};

type Scene = {
  positionBuffer: GPUBuffer;
  uvBuffer: GPUBuffer;
  indexBuffer: GPUBuffer;
  pipeline: GPURenderPipeline;
  bindGroup0: GPUBindGroup;
  bindGroup1: GPUBindGroup;
  indices: Accessor;
}[];

type Portal = {
  positionBuffer: GPUBuffer;
  indexBuffer: GPUBuffer;
  pipeline: GPURenderPipeline;
  bindGroup0: GPUBindGroup;
  indices: Accessor;
};

const createVertexBuffer = (device: GPUDevice, attribute: Accessor) => {
  const buffer = device.createBuffer({
    size: attribute.getByteLength(),
    usage: GPUBufferUsage.VERTEX,
    mappedAtCreation: true,
  });
  new Float32Array(buffer.getMappedRange()).set(attribute.getArray()!);
  buffer.unmap();
  return buffer;
};

const createIndexBuffer = (device: GPUDevice, attribute: Accessor) => {
  const buffer = device.createBuffer({
    size: attribute.getByteLength(),
    usage: GPUBufferUsage.INDEX,
    mappedAtCreation: true,
  });
  new Uint16Array(buffer.getMappedRange()).set(attribute.getArray()!);
  buffer.unmap();
  return buffer;
};

const createRenderPipeline = ({
  device,
  position,
  uvs,
  format,
  module,
  stencilFront,
}: {
  device: GPUDevice;
  position: Accessor;
  uvs: Accessor;
  format: GPUTextureFormat;
  module: GPUShaderModule;
  stencilFront: GPUStencilFaceState;
}) => {
  return device.createRenderPipeline({
    layout: 'auto',
    vertex: {
      entryPoint: 'v_main',
      module,
      buffers: [
        {
          stepMode: 'vertex',
          arrayStride: position.getElementSize() * position.getComponentSize(),
          attributes: [
            {
              // position
              format: 'float32x3',
              offset: 0,
              shaderLocation: 0,
            },
          ],
        },
        {
          stepMode: 'vertex',
          arrayStride: uvs.getElementSize() * uvs.getComponentSize(),
          attributes: [
            {
              // uvs
              format: 'float32x2',
              offset: 0,
              shaderLocation: 1,
            },
          ],
        },
      ],
    },
    fragment: {
      entryPoint: 'f_main',
      module,
      targets: [{format}],
    },
    primitive: {
      cullMode: 'back',
    },
    depthStencil: {
      depthWriteEnabled: true,
      depthCompare: 'less',
      format: 'depth24plus-stencil8',
      stencilFront,
    },
  });
};

const createPortalRenderPipeline = ({
  device,
  position,
  format,
  module,
  stencilFront,
}: {
  device: GPUDevice;
  position: Accessor;
  format: GPUTextureFormat;
  module: GPUShaderModule;
  stencilFront: GPUStencilFaceState;
}) => {
  return device.createRenderPipeline({
    layout: 'auto',
    vertex: {
      entryPoint: 'v_portal',
      module,
      buffers: [
        {
          stepMode: 'vertex',
          arrayStride: position.getElementSize() * position.getComponentSize(),
          attributes: [
            {
              // position
              format: 'float32x3',
              offset: 0,
              shaderLocation: 0,
            },
          ],
        },
      ],
    },
    fragment: {
      entryPoint: 'f_portal',
      module,
      targets: [{format}],
    },
    depthStencil: {
      format: 'depth24plus-stencil8',
      stencilFront,
      stencilBack: {
        compare: 'never',
      },
    },
  });
};

const createBindGroup0 = (
  device: GPUDevice,
  pipeline: GPURenderPipeline,
  buffer: GPUBuffer,
) => {
  return device.createBindGroup({
    layout: pipeline.getBindGroupLayout(0),
    entries: [
      {
        binding: 0,
        resource: {
          buffer,
        },
      },
    ],
  });
};

const createBindGroup1 = (
  device: GPUDevice,
  pipeline: GPURenderPipeline,
  sampler: GPUSampler,
  texture: GPUTexture,
) => {
  return device.createBindGroup({
    layout: pipeline.getBindGroupLayout(1),
    entries: [
      {
        binding: 0,
        resource: sampler,
      },
      {
        binding: 1,
        resource: texture.createView(),
      },
    ],
  });
};

const renderScene = ({
  device,
  scene,
  texture,
  depthStencilAttachment,
}: {
  device: GPUDevice;
  scene: Scene;
  texture: GPUTexture;
  depthStencilAttachment: GPURenderPassDepthStencilAttachment;
}) => {
  const encoder = device.createCommandEncoder();

  scene.forEach(
    ({
      bindGroup0,
      bindGroup1,
      indexBuffer,
      pipeline,
      positionBuffer,
      indices,
      uvBuffer,
    }) => {
      let pass = encoder.beginRenderPass({
        colorAttachments: [
          {
            view: texture.createView(),
            loadOp: 'load',
            storeOp: 'store',
          },
        ],
        depthStencilAttachment,
      });
      pass.setStencilReference(1);
      pass.setPipeline(pipeline);
      pass.setVertexBuffer(0, positionBuffer);
      pass.setVertexBuffer(1, uvBuffer);
      pass.setIndexBuffer(indexBuffer, 'uint16');
      pass.setBindGroup(0, bindGroup0);
      pass.setBindGroup(1, bindGroup1);
      pass.drawIndexed(indices.getCount());
      pass.end();
    },
  );

  device.queue.submit([encoder.finish()]);
};

const renderPortal = ({
  device,
  portal,
  texture,
  depthStencilAttachment,
}: {
  device: GPUDevice;
  portal: Portal;
  texture: GPUTexture;
  depthStencilAttachment: GPURenderPassDepthStencilAttachment;
}) => {
  const encoder = device.createCommandEncoder();
  const {bindGroup0, indexBuffer, pipeline, positionBuffer, indices} = portal;

  let pass = encoder.beginRenderPass({
    colorAttachments: [
      {
        view: texture.createView(),
        loadOp: 'load',
        storeOp: 'store',
      },
    ],
    depthStencilAttachment,
  });
  pass.setStencilReference(1);
  pass.setPipeline(pipeline);
  pass.setVertexBuffer(0, positionBuffer);
  pass.setIndexBuffer(indexBuffer, 'uint16');
  pass.setBindGroup(0, bindGroup0);
  pass.drawIndexed(indices.getCount());
  pass.end();

  device.queue.submit([encoder.finish()]);
};
