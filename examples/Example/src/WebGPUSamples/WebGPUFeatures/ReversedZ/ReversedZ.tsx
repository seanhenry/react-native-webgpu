/* eslint-disable @typescript-eslint/no-shadow */
import {mat4, Mat4, vec3} from 'wgpu-matrix';
import vertexWGSL from './vertex.wgsl';
import fragmentWGSL from './fragment.wgsl';
import vertexDepthPrePassWGSL from './vertexDepthPrePass.wgsl';
import vertexTextureQuadWGSL from './vertexTextureQuad.wgsl';
import fragmentTextureQuadWGSL from './fragmentTextureQuad.wgsl';
import vertexPrecisionErrorPassWGSL from './vertexPrecisionErrorPass.wgsl';
import fragmentPrecisionErrorPassWGSL from './fragmentPrecisionErrorPass.wgsl';
import {Square} from '../../../Components/Square';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../../../Components/globalStyles';
import {useControls} from '../../../Components/controls/react/useControls';
import {examplesCallback} from '../../../utils/examplesCallback';

export const ReversedZ = () => {
  const {gui, Controls} = useControls();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    requestAnimationFrame,
    navigator,
    context,
  }) => {
    // Two planes close to each other for depth precision test
    const geometryVertexSize = 4 * 8; // Byte size of one geometry vertex.
    const geometryPositionOffset = 0;
    const geometryColorOffset = 4 * 4; // Byte offset of geometry vertex color attribute.
    const geometryDrawCount = 6 * 2;

    const d = 0.0001; // half distance between two planes
    const o = 0.5; // half x offset to shift planes so they are only partially overlaping

    // prettier-ignore
    const geometryVertexArray = new Float32Array([
    // float4 position, float4 color
    -1 - o, -1, d, 1, 1, 0, 0, 1,
    1 - o, -1, d, 1, 1, 0, 0, 1,
    -1 - o, 1, d, 1, 1, 0, 0, 1,
    1 - o, -1, d, 1, 1, 0, 0, 1,
    1 - o, 1, d, 1, 1, 0, 0, 1,
    -1 - o, 1, d, 1, 1, 0, 0, 1,

    -1 + o, -1, -d, 1, 0, 1, 0, 1,
    1 + o, -1, -d, 1, 0, 1, 0, 1,
    -1 + o, 1, -d, 1, 0, 1, 0, 1,
    1 + o, -1, -d, 1, 0, 1, 0, 1,
    1 + o, 1, -d, 1, 0, 1, 0, 1,
    -1 + o, 1, -d, 1, 0, 1, 0, 1,
  ]);

    const xCount = 1;
    const yCount = 5;
    const numInstances = xCount * yCount;
    const matrixFloatCount = 16; // 4x4 matrix
    const matrixStride = 4 * matrixFloatCount; // 64;

    const depthRangeRemapMatrix = mat4.identity();
    depthRangeRemapMatrix[10] = -1;
    depthRangeRemapMatrix[14] = 1;

    enum DepthBufferMode {
      Default = 0,
      Reversed,
    }

    const depthBufferModes: DepthBufferMode[] = [
      DepthBufferMode.Default,
      DepthBufferMode.Reversed,
    ];
    const depthCompareFuncs = {
      [DepthBufferMode.Default]: 'less' as GPUCompareFunction,
      [DepthBufferMode.Reversed]: 'greater' as GPUCompareFunction,
    };
    const depthClearValues = {
      [DepthBufferMode.Default]: 1.0,
      [DepthBufferMode.Reversed]: 0.0,
    };

    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();

    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'auto',
    });

    const verticesBuffer = device.createBuffer({
      size: geometryVertexArray.byteLength,
      usage: GPUBufferUsage.VERTEX,
      mappedAtCreation: true,
    });
    new Float32Array(verticesBuffer.getMappedRange()).set(geometryVertexArray);
    verticesBuffer.unmap();

    const depthBufferFormat = 'depth32float';

    const depthTextureBindGroupLayout = device.createBindGroupLayout({
      entries: [
        {
          binding: 0,
          visibility: GPUShaderStage.FRAGMENT,
          texture: {
            sampleType: 'depth',
          },
        },
      ],
    });

    // Model, view, projection matrices
    const uniformBindGroupLayout = device.createBindGroupLayout({
      entries: [
        {
          binding: 0,
          visibility: GPUShaderStage.VERTEX,
          buffer: {
            type: 'uniform',
          },
        },
        {
          binding: 1,
          visibility: GPUShaderStage.VERTEX,
          buffer: {
            type: 'uniform',
          },
        },
      ],
    });

    const depthPrePassRenderPipelineLayout = device.createPipelineLayout({
      bindGroupLayouts: [uniformBindGroupLayout],
    });

    // depthPrePass is used to render scene to the depth texture
    // this is not needed if you just want to use reversed z to render a scene
    const depthPrePassRenderPipelineDescriptorBase = {
      layout: depthPrePassRenderPipelineLayout,
      vertex: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: vertexDepthPrePassWGSL,
        }),
        buffers: [
          {
            arrayStride: geometryVertexSize,
            attributes: [
              {
                // position
                shaderLocation: 0,
                offset: geometryPositionOffset,
                format: 'float32x4',
              },
            ],
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
        cullMode: 'back',
      },
      depthStencil: {
        depthWriteEnabled: true,
        depthCompare: 'less',
        format: depthBufferFormat,
      },
    } as GPURenderPipelineDescriptor;

    // we need the depthCompare to fit the depth buffer mode we are using.
    // this is the same for other passes
    const depthPrePassPipelines: GPURenderPipeline[] = [];
    depthPrePassRenderPipelineDescriptorBase.depthStencil!.depthCompare =
      depthCompareFuncs[DepthBufferMode.Default];
    depthPrePassPipelines[DepthBufferMode.Default] =
      device.createRenderPipeline(depthPrePassRenderPipelineDescriptorBase);
    depthPrePassRenderPipelineDescriptorBase.depthStencil!.depthCompare =
      depthCompareFuncs[DepthBufferMode.Reversed];
    depthPrePassPipelines[DepthBufferMode.Reversed] =
      device.createRenderPipeline(depthPrePassRenderPipelineDescriptorBase);

    // precisionPass is to draw precision error as color of depth value stored in depth buffer
    // compared to that directly calcualated in the shader
    const precisionPassRenderPipelineLayout = device.createPipelineLayout({
      bindGroupLayouts: [uniformBindGroupLayout, depthTextureBindGroupLayout],
    });
    const precisionPassRenderPipelineDescriptorBase = {
      layout: precisionPassRenderPipelineLayout,
      vertex: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: vertexPrecisionErrorPassWGSL,
        }),
        buffers: [
          {
            arrayStride: geometryVertexSize,
            attributes: [
              {
                // position
                shaderLocation: 0,
                offset: geometryPositionOffset,
                format: 'float32x4',
              },
            ],
          },
        ],
      },
      fragment: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: fragmentPrecisionErrorPassWGSL,
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
      depthStencil: {
        depthWriteEnabled: true,
        depthCompare: 'less',
        format: depthBufferFormat,
      },
    } as GPURenderPipelineDescriptor;
    const precisionPassPipelines: GPURenderPipeline[] = [];
    precisionPassRenderPipelineDescriptorBase.depthStencil!.depthCompare =
      depthCompareFuncs[DepthBufferMode.Default];
    precisionPassPipelines[DepthBufferMode.Default] =
      device.createRenderPipeline(precisionPassRenderPipelineDescriptorBase);
    precisionPassRenderPipelineDescriptorBase.depthStencil!.depthCompare =
      depthCompareFuncs[DepthBufferMode.Reversed];
    // prettier-ignore
    precisionPassPipelines[DepthBufferMode.Reversed] = device.createRenderPipeline(
    precisionPassRenderPipelineDescriptorBase,
  );

    // colorPass is the regular render pass to render the scene
    const colorPassRenderPiplineLayout = device.createPipelineLayout({
      bindGroupLayouts: [uniformBindGroupLayout],
    });
    const colorPassRenderPipelineDescriptorBase: GPURenderPipelineDescriptor = {
      layout: colorPassRenderPiplineLayout,
      vertex: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: vertexWGSL,
        }),
        buffers: [
          {
            arrayStride: geometryVertexSize,
            attributes: [
              {
                // position
                shaderLocation: 0,
                offset: geometryPositionOffset,
                format: 'float32x4',
              },
              {
                // color
                shaderLocation: 1,
                offset: geometryColorOffset,
                format: 'float32x4',
              },
            ],
          },
        ],
      },
      fragment: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: fragmentWGSL,
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
      depthStencil: {
        depthWriteEnabled: true,
        depthCompare: 'less',
        format: depthBufferFormat,
      },
    };
    const colorPassPipelines: GPURenderPipeline[] = [];
    colorPassRenderPipelineDescriptorBase.depthStencil!.depthCompare =
      depthCompareFuncs[DepthBufferMode.Default];
    colorPassPipelines[DepthBufferMode.Default] = device.createRenderPipeline(
      colorPassRenderPipelineDescriptorBase,
    );
    colorPassRenderPipelineDescriptorBase.depthStencil!.depthCompare =
      depthCompareFuncs[DepthBufferMode.Reversed];
    colorPassPipelines[DepthBufferMode.Reversed] = device.createRenderPipeline(
      colorPassRenderPipelineDescriptorBase,
    );

    // textureQuadPass is draw a full screen quad of depth texture
    // to see the difference of depth value using reversed z compared to default depth buffer usage
    // 0.0 will be the furthest and 1.0 will be the closest
    const textureQuadPassPiplineLayout = device.createPipelineLayout({
      bindGroupLayouts: [depthTextureBindGroupLayout],
    });
    const textureQuadPassPipline = device.createRenderPipeline({
      layout: textureQuadPassPiplineLayout,
      vertex: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: vertexTextureQuadWGSL,
        }),
      },
      fragment: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: fragmentTextureQuadWGSL,
        }),
        targets: [
          {
            format: presentationFormat,
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
      },
    });

    const depthTexture = device.createTexture({
      size: [context.width, context.height],
      format: depthBufferFormat,
      usage:
        GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING,
    });
    const depthTextureView = depthTexture.createView();

    const defaultDepthTexture = device.createTexture({
      size: [context.width, context.height],
      format: depthBufferFormat,
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
    });
    const defaultDepthTextureView = defaultDepthTexture.createView();

    const depthPrePassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [],
      depthStencilAttachment: {
        view: depthTextureView,

        depthClearValue: 1.0,
        depthLoadOp: 'clear',
        depthStoreOp: 'store',
      },
    };

    // drawPassDescriptor and drawPassLoadDescriptor are used for drawing
    // the scene twice using different depth buffer mode on splitted viewport
    // of the same canvas
    // see the difference of the loadOp of the colorAttachments
    const drawPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          // view is acquired and set in render loop.
          view: undefined as unknown as GPUTextureView,

          clearValue: [0.0, 0.0, 0.5, 1.0],
          loadOp: 'clear',
          storeOp: 'store',
        },
      ],
      depthStencilAttachment: {
        view: defaultDepthTextureView,

        depthClearValue: 1.0,
        depthLoadOp: 'clear',
        depthStoreOp: 'store',
      },
    };
    const drawPassLoadDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          // attachment is acquired and set in render loop.
          view: undefined as unknown as GPUTextureView,

          loadOp: 'load',
          storeOp: 'store',
        },
      ],
      depthStencilAttachment: {
        view: defaultDepthTextureView,

        depthClearValue: 1.0,
        depthLoadOp: 'clear',
        depthStoreOp: 'store',
      },
    };
    const drawPassDescriptors = [drawPassDescriptor, drawPassLoadDescriptor];

    const textureQuadPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          // view is acquired and set in render loop.
          view: undefined as unknown as GPUTextureView,

          clearValue: [0.0, 0.0, 0.5, 1.0],
          loadOp: 'clear',
          storeOp: 'store',
        },
      ],
    };
    const textureQuadPassLoadDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          // view is acquired and set in render loop.
          view: undefined as unknown as GPUTextureView,

          loadOp: 'load',
          storeOp: 'store',
        },
      ],
    };
    const textureQuadPassDescriptors = [
      textureQuadPassDescriptor,
      textureQuadPassLoadDescriptor,
    ];

    const depthTextureBindGroup = device.createBindGroup({
      layout: depthTextureBindGroupLayout,
      entries: [
        {
          binding: 0,
          resource: depthTextureView,
        },
      ],
    });

    const uniformBufferSize = numInstances * matrixStride;

    const uniformBuffer = device.createBuffer({
      size: uniformBufferSize,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
    const cameraMatrixBuffer = device.createBuffer({
      size: 4 * 16, // 4x4 matrix
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
    const cameraMatrixReversedDepthBuffer = device.createBuffer({
      size: 4 * 16, // 4x4 matrix
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    const uniformBindGroups = [
      device.createBindGroup({
        layout: uniformBindGroupLayout,
        entries: [
          {
            binding: 0,
            resource: {
              buffer: uniformBuffer,
            },
          },
          {
            binding: 1,
            resource: {
              buffer: cameraMatrixBuffer,
            },
          },
        ],
      }),
      device.createBindGroup({
        layout: uniformBindGroupLayout,
        entries: [
          {
            binding: 0,
            resource: {
              buffer: uniformBuffer,
            },
          },
          {
            binding: 1,
            resource: {
              buffer: cameraMatrixReversedDepthBuffer,
            },
          },
        ],
      }),
    ];

    const modelMatrices = new Array<Mat4>(numInstances);
    const mvpMatricesData = new Float32Array(matrixFloatCount * numInstances);

    let m = 0;
    for (let x = 0; x < xCount; x++) {
      for (let y = 0; y < yCount; y++) {
        const z = -800 * m;
        const s = 1 + 50 * m;

        modelMatrices[m] = mat4.translation(
          vec3.fromValues(
            x - xCount / 2 + 0.5,
            (4.0 - 0.2 * z) * (y - yCount / 2 + 1.0),
            z,
          ),
        );
        mat4.scale(
          modelMatrices[m],
          vec3.fromValues(s, s, s),
          modelMatrices[m],
        );

        m++;
      }
    }

    const viewMatrix = mat4.translation(vec3.fromValues(0, 0, -12));

    const aspect = (0.5 * context.width) / context.height;
    // wgpu-matrix perspective doesn't handle zFar === Infinity now.
    // https://github.com/greggman/wgpu-matrix/issues/9
    const projectionMatrix = mat4.perspective(
      (2 * Math.PI) / 5,
      aspect,
      5,
      9999,
    );

    const viewProjectionMatrix = mat4.multiply(projectionMatrix, viewMatrix);
    // to use 1/z we just multiple depthRangeRemapMatrix to our default camera view projection matrix
    const reversedRangeViewProjectionMatrix = mat4.multiply(
      depthRangeRemapMatrix,
      viewProjectionMatrix,
    );

    device.queue.writeBuffer(cameraMatrixBuffer, 0, viewProjectionMatrix);
    device.queue.writeBuffer(
      cameraMatrixReversedDepthBuffer,
      0,
      reversedRangeViewProjectionMatrix,
    );

    const tmpMat4 = mat4.create();

    function updateTransformationMatrix() {
      const now = Date.now() / 1000;

      for (let i = 0, m = 0; i < numInstances; i++, m += matrixFloatCount) {
        mat4.rotate(
          modelMatrices[i],
          vec3.fromValues(Math.sin(now), Math.cos(now), 0),
          (Math.PI / 180) * 30,
          tmpMat4,
        );
        mvpMatricesData.set(tmpMat4, m);
      }
    }

    const settings = {
      mode: 'color',
    };
    gui.add(settings, 'mode', ['color', 'precision-error', 'depth-texture']);
    gui.draw();

    function frame() {
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }
      updateTransformationMatrix();
      device.queue.writeBuffer(
        uniformBuffer,
        0,
        mvpMatricesData.buffer,
        mvpMatricesData.byteOffset,
        mvpMatricesData.byteLength,
      );

      const attachment = framebuffer.createView();
      const commandEncoder = device.createCommandEncoder();
      if (settings.mode === 'color') {
        for (const m of depthBufferModes) {
          (
            drawPassDescriptors[m]
              .colorAttachments as GPURenderPassColorAttachment[]
          )[0].view = attachment;
          (
            drawPassDescriptors[m]
              .depthStencilAttachment as GPURenderPassDepthStencilAttachment
          ).depthClearValue = depthClearValues[m];
          const colorPass = commandEncoder.beginRenderPass(
            drawPassDescriptors[m],
          );
          colorPass.setPipeline(colorPassPipelines[m]);
          colorPass.setBindGroup(0, uniformBindGroups[m]);
          colorPass.setVertexBuffer(0, verticesBuffer);
          colorPass.setViewport(
            (context.width * m) / 2,
            0,
            context.width / 2,
            context.height,
            0,
            1,
          );
          colorPass.draw(geometryDrawCount, numInstances, 0, 0);
          colorPass.end();
        }
      } else if (settings.mode === 'precision-error') {
        for (const m of depthBufferModes) {
          {
            (
              depthPrePassDescriptor.depthStencilAttachment as GPURenderPassDepthStencilAttachment
            ).depthClearValue = depthClearValues[m];
            const depthPrePass = commandEncoder.beginRenderPass(
              depthPrePassDescriptor,
            );
            depthPrePass.setPipeline(depthPrePassPipelines[m]);
            depthPrePass.setBindGroup(0, uniformBindGroups[m]);
            depthPrePass.setVertexBuffer(0, verticesBuffer);
            depthPrePass.setViewport(
              (context.width * m) / 2,
              0,
              context.width / 2,
              context.height,
              0,
              1,
            );
            depthPrePass.draw(geometryDrawCount, numInstances, 0, 0);
            depthPrePass.end();
          }
          {
            (
              drawPassDescriptors[m]
                .colorAttachments as GPURenderPassColorAttachment[]
            )[0].view = attachment;
            (
              drawPassDescriptors[m]
                .depthStencilAttachment as GPURenderPassDepthStencilAttachment
            ).depthClearValue = depthClearValues[m];
            const precisionErrorPass = commandEncoder.beginRenderPass(
              drawPassDescriptors[m],
            );
            precisionErrorPass.setPipeline(precisionPassPipelines[m]);
            precisionErrorPass.setBindGroup(0, uniformBindGroups[m]);
            precisionErrorPass.setBindGroup(1, depthTextureBindGroup);
            precisionErrorPass.setVertexBuffer(0, verticesBuffer);
            precisionErrorPass.setViewport(
              (context.width * m) / 2,
              0,
              context.width / 2,
              context.height,
              0,
              1,
            );
            precisionErrorPass.draw(geometryDrawCount, numInstances, 0, 0);
            precisionErrorPass.end();
          }
        }
      } else {
        // depth texture quad
        for (const m of depthBufferModes) {
          {
            (
              depthPrePassDescriptor.depthStencilAttachment as GPURenderPassDepthStencilAttachment
            ).depthClearValue = depthClearValues[m];
            const depthPrePass = commandEncoder.beginRenderPass(
              depthPrePassDescriptor,
            );
            depthPrePass.setPipeline(depthPrePassPipelines[m]);
            depthPrePass.setBindGroup(0, uniformBindGroups[m]);
            depthPrePass.setVertexBuffer(0, verticesBuffer);
            depthPrePass.setViewport(
              (context.width * m) / 2,
              0,
              context.width / 2,
              context.height,
              0,
              1,
            );
            depthPrePass.draw(geometryDrawCount, numInstances, 0, 0);
            depthPrePass.end();
          }
          {
            (
              textureQuadPassDescriptors[m]
                .colorAttachments as GPURenderPassColorAttachment[]
            )[0].view = attachment;
            const depthTextureQuadPass = commandEncoder.beginRenderPass(
              textureQuadPassDescriptors[m],
            );
            depthTextureQuadPass.setPipeline(textureQuadPassPipline);
            depthTextureQuadPass.setBindGroup(0, depthTextureBindGroup);
            depthTextureQuadPass.setViewport(
              (context.width * m) / 2,
              0,
              context.width / 2,
              context.height,
              0,
              1,
            );
            depthTextureQuadPass.draw(6);
            depthTextureQuadPass.end();
          }
        }
      }
      device.queue.submit([commandEncoder.finish()]);
      examplesCallback(device.queue);
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
