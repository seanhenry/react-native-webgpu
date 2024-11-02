import {Square} from '../../Components/Square';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../../Components/globalStyles';
import {mat4, vec3} from 'wgpu-matrix';
import {WebIO} from '@gltf-transform/core';
import modelGltf from './cube.gltf';
import outlinesWgsl from './outlines.wgsl';
import {useControls} from '../../Components/controls/react/useControls';

export const Outlines = () => {
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
    });

    // Load cube from GLTF
    const document = await new WebIO().readJSON({
      json: modelGltf,
      resources: {},
    });
    const cubeMesh = document
      .getRoot()
      .listMeshes()
      .find(mesh => mesh.getName() === 'Cube')!;
    const cubePrimitive = cubeMesh.listPrimitives()[0]!;
    const cubePosition = cubePrimitive.getAttribute('POSITION')!;
    const cubeIndices = cubePrimitive.getIndices()!;

    // Load cube position vertices
    const cubeBuffer = device.createBuffer({
      size: cubePosition.getByteLength(),
      usage: GPUBufferUsage.VERTEX,
      mappedAtCreation: true,
    });
    new Float32Array(cubeBuffer.getMappedRange()).set(cubePosition.getArray()!);
    cubeBuffer.unmap();

    // Load cube indices
    const cubeIndexBuffer = device.createBuffer({
      size: cubeIndices.getByteLength(),
      usage: GPUBufferUsage.INDEX,
      mappedAtCreation: true,
    });
    new Uint16Array(cubeIndexBuffer.getMappedRange()).set(
      cubeIndices.getArray()!,
    );
    cubeIndexBuffer.unmap();

    // Make uniforms buffer
    let viewProjectionMat = mat4.identity();
    const uniformsBuffer = device.createBuffer({
      size: viewProjectionMat.byteLength,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    // Pipeline
    const module = device.createShaderModule({
      code: outlinesWgsl,
    });
    const cubePipelineDescriptor: GPURenderPipelineDescriptor = {
      layout: 'auto',
      vertex: {
        entryPoint: 'v_main',
        module,
        buffers: [
          {
            stepMode: 'vertex',
            arrayStride:
              cubePosition.getElementSize() * cubePosition.getComponentSize(),
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
        entryPoint: 'f_cube',
        module,
        targets: [{format: preferredFormat}],
      },
      depthStencil: {
        format: 'depth24plus-stencil8',
        stencilFront: {
          compare: 'always',
          depthFailOp: 'replace',
          failOp: 'replace',
          passOp: 'replace',
        },
        stencilBack: {
          compare: 'never',
        },
      },
    };
    const outlinePipelineDescriptor: GPURenderPipelineDescriptor = {
      ...cubePipelineDescriptor,
      fragment: {
        entryPoint: 'f_outline',
        module,
        targets: [{format: preferredFormat}],
      },
      depthStencil: {
        format: 'depth24plus-stencil8',
        stencilFront: {
          compare: 'not-equal',
        },
        stencilBack: {
          compare: 'never',
        },
      },
    };
    const cubePipeline = device.createRenderPipeline(cubePipelineDescriptor);
    const outlinePipeline = device.createRenderPipeline(
      outlinePipelineDescriptor,
    );
    const cubeBindGroup0 = device.createBindGroup({
      layout: cubePipeline.getBindGroupLayout(0),
      entries: [
        {
          binding: 0,
          resource: {
            buffer: uniformsBuffer,
          },
        },
      ],
    });
    const outlineBindGroup0 = device.createBindGroup({
      layout: outlinePipeline.getBindGroupLayout(0),
      entries: [
        {
          binding: 0,
          resource: {
            buffer: uniformsBuffer,
          },
        },
      ],
    });

    // Depth and stencil texture
    const depthTexture = device.createTexture({
      size: context,
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
      format: 'depth24plus-stencil8',
    });
    const cubeDepthStencilAttachment: GPURenderPassDepthStencilAttachment = {
      view: depthTexture.createView(),
      depthLoadOp: 'clear',
      depthStoreOp: 'store',
      stencilLoadOp: 'clear',
      stencilStoreOp: 'store',
    };
    const outlineDepthStencilAttachment: GPURenderPassDepthStencilAttachment = {
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
    const cubeModelMat = mat4.identity();

    // GUI
    const params = {
      outlineScale: 1.2,
    };
    gui.add(params, 'outlineScale', 1, 2, 0.01);
    gui.draw();

    // @ts-expect-error missing type for performance
    let time = performance.now();

    const frame = (t: number) => {
      const dt = t - time;
      time = t;
      const texture = context.getCurrentTexture();
      if (!texture) {
        requestAnimationFrame(frame);
        return;
      }
      mat4.axisRotate(
        cubeModelMat,
        vec3.fromValues(1, 1, 0),
        dt * 0.001,
        cubeModelMat,
      );

      let encoder = device.createCommandEncoder();

      mat4.multiply(projectionMat, viewMat, viewProjectionMat);
      mat4.multiply(viewProjectionMat, cubeModelMat, viewProjectionMat);
      device.queue.writeBuffer(
        uniformsBuffer,
        0,
        viewProjectionMat.buffer,
        viewProjectionMat.byteOffset,
        viewProjectionMat.byteLength,
      );

      let pass = encoder.beginRenderPass({
        colorAttachments: [
          {
            view: texture.createView(),
            loadOp: 'clear',
            storeOp: 'store',
            clearValue: [0, 0, 0, 1],
          },
        ],
        depthStencilAttachment: cubeDepthStencilAttachment,
      });
      pass.setStencilReference(1);
      pass.setPipeline(cubePipeline);
      pass.setVertexBuffer(0, cubeBuffer);
      pass.setIndexBuffer(cubeIndexBuffer, 'uint16');
      pass.setBindGroup(0, cubeBindGroup0);
      pass.drawIndexed(cubeIndices.getCount());
      pass.end();

      device.queue.submit([encoder.finish()]);

      encoder = device.createCommandEncoder();

      const outlineModelMat = mat4.scale(
        cubeModelMat,
        vec3.fromValues(
          params.outlineScale,
          params.outlineScale,
          params.outlineScale,
        ),
      );
      mat4.multiply(projectionMat, viewMat, viewProjectionMat);
      mat4.multiply(viewProjectionMat, outlineModelMat, viewProjectionMat);
      device.queue.writeBuffer(
        uniformsBuffer,
        0,
        viewProjectionMat.buffer,
        viewProjectionMat.byteOffset,
        viewProjectionMat.byteLength,
      );

      pass = encoder.beginRenderPass({
        colorAttachments: [
          {
            view: texture.createView(),
            loadOp: 'load',
            storeOp: 'store',
          },
        ],
        depthStencilAttachment: outlineDepthStencilAttachment,
      });
      pass.setStencilReference(1);
      pass.setPipeline(outlinePipeline);
      pass.setVertexBuffer(0, cubeBuffer);
      pass.setIndexBuffer(cubeIndexBuffer, 'uint16');
      pass.setBindGroup(0, outlineBindGroup0);
      pass.drawIndexed(cubeIndices.getCount());
      pass.end();

      device.queue.submit([encoder.finish()]);
      context.presentSurface();
      requestAnimationFrame(frame);
    };
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
