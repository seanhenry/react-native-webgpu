import {Square} from '../../../Components/Square';
import {WebGpuView, type WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../../../Components/globalStyles';

import spriteWGSL from './sprite.wgsl';
import updateSpritesWGSL from './updateSprites.wgsl';
import {useControls} from '../../../Components/controls/react/useControls';
import {HudContainer} from '../../../Components/stats/HudContainer';
import {useHudText} from '../../../Components/stats/useHudText';
import {examplesCallback} from '../../../utils/examplesCallback';

export const ComputeBoids = () => {
  const {gui, Controls} = useControls();
  const {setText, HudText} = useHudText();

  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    context,
    navigator,
    requestAnimationFrame,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();

    const hasTimestampQuery = adapter!.features.has('timestamp-query');
    const device = await adapter!.requestDevice({
      requiredFeatures: hasTimestampQuery ? ['timestamp-query'] : [],
    });

    setText(
      hasTimestampQuery
        ? 'Collecting samples...'
        : 'timestamp-query not supported on this device',
    );

    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'auto',
    });

    const spriteShaderModule = device.createShaderModule({code: spriteWGSL});
    const renderPipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        module: spriteShaderModule,
        entryPoint: 'vert_main',
        buffers: [
          {
            // instanced particles buffer
            arrayStride: 4 * 4,
            stepMode: 'instance',
            attributes: [
              {
                // instance position
                shaderLocation: 0,
                offset: 0,
                format: 'float32x2',
              },
              {
                // instance velocity
                shaderLocation: 1,
                offset: 2 * 4,
                format: 'float32x2',
              },
            ],
          },
          {
            // vertex buffer
            arrayStride: 2 * 4,
            stepMode: 'vertex',
            attributes: [
              {
                // vertex positions
                shaderLocation: 2,
                offset: 0,
                format: 'float32x2',
              },
            ],
          },
        ],
      },
      fragment: {
        module: spriteShaderModule,
        entryPoint: 'frag_main',
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

    const computePipeline = device.createComputePipeline({
      layout: 'auto',
      compute: {
        module: device.createShaderModule({
          code: updateSpritesWGSL,
        }),
        entryPoint: 'main',
      },
    });

    const renderPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          view: undefined as unknown as GPUTextureView, // Assigned later
          clearValue: [0, 0, 0, 1],
          loadOp: 'clear' as const,
          storeOp: 'store' as const,
        },
      ],
    };

    const computePassDescriptor: GPUComputePassDescriptor = {};

    /** Storage for timestamp query results */
    let querySet: GPUQuerySet | undefined;
    /** Timestamps are resolved into this buffer */
    let resolveBuffer: GPUBuffer | undefined;
    /** Pool of spare buffers for MAP_READing the timestamps back to CPU. A buffer
     * is taken from the pool (if available) when a readback is needed, and placed
     * back into the pool once the readback is done and it's unmapped. */
    const spareResultBuffers: GPUBuffer[] = [];

    if (hasTimestampQuery) {
      querySet = device.createQuerySet({
        type: 'timestamp',
        count: 4,
      });
      resolveBuffer = device.createBuffer({
        size: 4 * BigInt64Array.BYTES_PER_ELEMENT,
        usage: GPUBufferUsage.QUERY_RESOLVE | GPUBufferUsage.COPY_SRC,
      });
      computePassDescriptor.timestampWrites = {
        querySet: querySet!,
        beginningOfPassWriteIndex: 0,
        endOfPassWriteIndex: 1,
      };
      renderPassDescriptor.timestampWrites = {
        querySet: querySet!,
        beginningOfPassWriteIndex: 2,
        endOfPassWriteIndex: 3,
      };
    }

    // prettier-ignore
    const vertexBufferData = new Float32Array([
      -0.01, -0.02, 0.01,
      -0.02, 0.0, 0.02,
    ]);

    const spriteVertexBuffer = device.createBuffer({
      size: vertexBufferData.byteLength,
      usage: GPUBufferUsage.VERTEX,
      mappedAtCreation: true,
    });
    new Float32Array(spriteVertexBuffer.getMappedRange()).set(vertexBufferData);
    spriteVertexBuffer.unmap();

    const simParams = {
      deltaT: 0.04,
      rule1Distance: 0.1,
      rule2Distance: 0.025,
      rule3Distance: 0.025,
      rule1Scale: 0.02,
      rule2Scale: 0.05,
      rule3Scale: 0.005,
    };

    const simParamBufferSize = 7 * Float32Array.BYTES_PER_ELEMENT;
    const simParamBuffer = device.createBuffer({
      size: simParamBufferSize,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    function updateSimParams() {
      device.queue.writeBuffer(
        simParamBuffer,
        0,
        new Float32Array([
          simParams.deltaT,
          simParams.rule1Distance,
          simParams.rule2Distance,
          simParams.rule3Distance,
          simParams.rule1Scale,
          simParams.rule2Scale,
          simParams.rule3Scale,
        ]),
      );
    }

    updateSimParams();
    Object.keys(simParams).forEach(k => {
      const key = k as keyof typeof simParams;
      gui.add(simParams, key).onFinishChange(updateSimParams);
    });
    gui.draw();

    const numParticles = 1500;
    const initialParticleData = new Float32Array(numParticles * 4);
    for (let i = 0; i < numParticles; ++i) {
      initialParticleData[4 * i + 0] = 2 * (Math.random() - 0.5);
      initialParticleData[4 * i + 1] = 2 * (Math.random() - 0.5);
      initialParticleData[4 * i + 2] = 2 * (Math.random() - 0.5) * 0.1;
      initialParticleData[4 * i + 3] = 2 * (Math.random() - 0.5) * 0.1;
    }

    const particleBuffers: GPUBuffer[] = new Array(2);
    const particleBindGroups: GPUBindGroup[] = new Array(2);
    for (let i = 0; i < 2; ++i) {
      particleBuffers[i] = device.createBuffer({
        size: initialParticleData.byteLength,
        usage: GPUBufferUsage.VERTEX | GPUBufferUsage.STORAGE,
        mappedAtCreation: true,
      });
      new Float32Array(particleBuffers[i]!.getMappedRange()).set(
        initialParticleData,
      );
      particleBuffers[i]!.unmap();
    }

    for (let i = 0; i < 2; ++i) {
      particleBindGroups[i] = device.createBindGroup({
        layout: computePipeline.getBindGroupLayout(0),
        entries: [
          {
            binding: 0,
            resource: {
              buffer: simParamBuffer,
            },
          },
          {
            binding: 1,
            resource: {
              buffer: particleBuffers[i]!,
              offset: 0,
              size: initialParticleData.byteLength,
            },
          },
          {
            binding: 2,
            resource: {
              buffer: particleBuffers[(i + 1) % 2]!,
              offset: 0,
              size: initialParticleData.byteLength,
            },
          },
        ],
      });
    }

    let t = 0;
    let computePassDurationSum = 0;
    let renderPassDurationSum = 0;
    let timerSamples = 0;
    function frame() {
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }
      (
        renderPassDescriptor.colorAttachments as GPURenderPassColorAttachment[]
      )[0]!.view = framebuffer.createView();

      const commandEncoder = device.createCommandEncoder();
      {
        const passEncoder = commandEncoder.beginComputePass(
          computePassDescriptor,
        );
        passEncoder.setPipeline(computePipeline);
        passEncoder.setBindGroup(0, particleBindGroups[t % 2]!);
        passEncoder.dispatchWorkgroups(Math.ceil(numParticles / 64));
        passEncoder.end();
      }
      {
        const passEncoder =
          commandEncoder.beginRenderPass(renderPassDescriptor);
        passEncoder.setPipeline(renderPipeline);
        passEncoder.setVertexBuffer(0, particleBuffers[(t + 1) % 2]!);
        passEncoder.setVertexBuffer(1, spriteVertexBuffer);
        passEncoder.draw(3, numParticles, 0, 0);
        passEncoder.end();
      }

      let resultBuffer: GPUBuffer | undefined;
      if (hasTimestampQuery) {
        resultBuffer =
          spareResultBuffers.pop() ||
          device.createBuffer({
            size: 4 * BigInt64Array.BYTES_PER_ELEMENT,
            usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.MAP_READ,
          });
        commandEncoder.resolveQuerySet(querySet!, 0, 4, resolveBuffer!, 0);
        commandEncoder.copyBufferToBuffer(
          resolveBuffer!,
          0,
          resultBuffer,
          0,
          resultBuffer!.size,
        );
      }

      device.queue.submit([commandEncoder.finish()]);
      examplesCallback(device.queue);

      context.presentSurface();

      if (hasTimestampQuery) {
        resultBuffer!.mapAsync(GPUMapMode.READ).then(() => {
          const times = new BigInt64Array(resultBuffer!.getMappedRange());
          const computePassDuration = Number(times[1]! - times[0]!);
          const renderPassDuration = Number(times[3]! - times[2]!);

          // In some cases the timestamps may wrap around and produce a negative
          // number as the GPU resets it's timings. These can safely be ignored.
          if (computePassDuration > 0 && renderPassDuration > 0) {
            computePassDurationSum += computePassDuration;
            renderPassDurationSum += renderPassDuration;
            timerSamples++;
          }
          resultBuffer!.unmap();

          // Periodically update the text for the timer stats
          const kNumTimerSamplesPerUpdate = 10;
          if (timerSamples >= kNumTimerSamplesPerUpdate) {
            const avgComputeMicroseconds = Math.round(
              computePassDurationSum / timerSamples / 1000,
            );
            const avgRenderMicroseconds = Math.round(
              renderPassDurationSum / timerSamples / 1000,
            );
            setText(`\
avg compute pass duration: ${avgComputeMicroseconds}µs
avg render pass duration:  ${avgRenderMicroseconds}µs
spare readback buffers:    ${spareResultBuffers.length}`);
            computePassDurationSum = 0;
            renderPassDurationSum = 0;
            timerSamples = 0;
          }
          spareResultBuffers.push(resultBuffer!);
        });
      }

      ++t;
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
      <HudContainer>
        <HudText />
      </HudContainer>
      <Controls />
    </>
  );
};
