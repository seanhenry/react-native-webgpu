import fullscreenTexturedQuad from '../../shaders/fullscreenTexturedQuad.wgsl';
import type {SurfaceBackedWebGPU, WGPUContext} from 'react-native-webgpu';
import {useControls} from '../../../Components/controls/react/useControls';
import {useStats} from '../../../Components/stats/useStats';

type BindGroupBindingLayout = GPUBufferBindingLayout;
//   | GPUTextureBindingLayout
//   | GPUSamplerBindingLayout
//   | GPUStorageTextureBindingLayout
//   | GPUExternalTextureBindingLayout;

// An object containing
// 1. A generated Bind Group Layout
// 2. An array of Bind Groups that accord to that layout
export type BindGroupCluster = {
  bindGroups: GPUBindGroup[];
  bindGroupLayout: GPUBindGroupLayout;
};

type ResourceTypeName = 'buffer';
// | 'texture'
// | 'sampler'
// | 'externalTexture'
// | 'storageTexture';

/**
 * @param {number[]} bindings - The binding value of each resource in the bind group.
 * @param {number[]} visibilities - The GPUShaderStage visibility of the resource at the corresponding index.
 * @param {ResourceTypeName[]} resourceTypes - The resourceType at the corresponding index.
 * @returns {BindGroupsObjectsAndLayout} An object containing an array of bindGroups and the bindGroupLayout they implement.
 */
export const createBindGroupCluster = (
  bindings: number[],
  visibilities: number[],
  resourceTypes: ResourceTypeName[],
  resourceLayouts: BindGroupBindingLayout[],
  resources: GPUBindingResource[][],
  label: string,
  device: GPUDevice,
): BindGroupCluster => {
  const layoutEntries: GPUBindGroupLayoutEntry[] = [];
  for (let i = 0; i < bindings.length; i++) {
    layoutEntries.push({
      binding: bindings[i]!,
      visibility: visibilities[i % visibilities.length]!,
      [resourceTypes[i]!]: resourceLayouts[i],
    });
  }

  const bindGroupLayout = device.createBindGroupLayout({
    label: `${label}.bindGroupLayout`,
    entries: layoutEntries,
  });

  const bindGroups: GPUBindGroup[] = [];
  //i represent the bindGroup index, j represents the binding index of the resource within the bindgroup
  //i=0, j=0  bindGroup: 0, binding: 0
  //i=1, j=1, bindGroup: 0, binding: 1
  //NOTE: not the same as @group(0) @binding(1) group index within the fragment shader is set within a pipeline
  for (let i = 0; i < resources.length; i++) {
    const groupEntries: GPUBindGroupEntry[] = [];
    for (let j = 0; j < resources[0]!.length; j++) {
      groupEntries.push({
        binding: j,
        resource: resources[i]![j]!,
      });
    }
    const newBindGroup = device.createBindGroup({
      label: `${label}.bindGroup${i}`,
      layout: bindGroupLayout,
      entries: groupEntries,
    });
    bindGroups.push(newBindGroup);
  }

  return {
    bindGroups,
    bindGroupLayout,
  };
};

export type ShaderKeyInterface<T extends string[]> = {
  [K in T[number]]: number;
};

export type SampleInitParams = {
  context: WGPUContext;
  navigator: SurfaceBackedWebGPU['navigator'];
  gui: ReturnType<typeof useControls>['gui'];
  stats: ReturnType<typeof useStats>['stats'];
};

interface DeviceInitParms {
  device: GPUDevice;
}

interface DeviceInit3DParams extends DeviceInitParms {
  context: WGPUContext;
  presentationFormat: GPUTextureFormat;
  timestampQueryAvailable: boolean;
}

type CallbackSync3D = (params: SampleInitParams & DeviceInit3DParams) => void;
type CallbackAsync3D = (
  params: SampleInitParams & DeviceInit3DParams,
) => Promise<void>;

type SampleInitCallback3D = CallbackSync3D | CallbackAsync3D;
export type SampleInit = (params: SampleInitParams) => void;

export const SampleInitFactoryWebGPU = async (
  callback: SampleInitCallback3D,
): Promise<SampleInit> => {
  const init = async ({context, navigator, gui, stats}: SampleInitParams) => {
    const adapter = await navigator.gpu.requestAdapter();
    const timestampQueryAvailable = adapter!.features.has('timestamp-query');
    let device: GPUDevice;
    if (timestampQueryAvailable) {
      device = await adapter!.requestDevice({
        requiredFeatures: ['timestamp-query'],
      });
    } else {
      device = await adapter!.requestDevice();
    }
    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();
    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'auto',
    });

    callback({
      gui,
      navigator,
      device,
      context,
      presentationFormat,
      stats,
      timestampQueryAvailable,
    });
  };
  return init;
};

export abstract class Base2DRendererClass {
  abstract switchBindGroup(name: string): void;
  abstract startRun(
    commandEncoder: GPUCommandEncoder,
    ...args: unknown[]
  ): void;
  // @ts-expect-error fix strict error
  renderPassDescriptor: GPURenderPassDescriptor;
  // @ts-expect-error fix strict error
  pipeline: GPURenderPipeline;
  // @ts-expect-error fix strict error
  bindGroupMap: Record<string, GPUBindGroup>;
  // @ts-expect-error fix strict error
  currentBindGroup: GPUBindGroup;
  // @ts-expect-error fix strict error
  currentBindGroupName: string;

  executeRun(
    commandEncoder: GPUCommandEncoder,
    renderPassDescriptor: GPURenderPassDescriptor,
    pipeline: GPURenderPipeline,
    bindGroups: GPUBindGroup[],
  ) {
    const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
    passEncoder.setPipeline(pipeline);
    for (let i = 0; i < bindGroups.length; i++) {
      passEncoder.setBindGroup(i, bindGroups[i]!);
    }
    passEncoder.draw(6, 1, 0, 0);
    passEncoder.end();
  }

  setUniformArguments<
    T extends {[key: string]: number},
    K extends readonly string[],
  >(device: GPUDevice, uniformBuffer: GPUBuffer, instance: T, keys: K) {
    for (let i = 0; i < keys.length; i++) {
      device.queue.writeBuffer(
        uniformBuffer,
        i * 4,
        new Float32Array([instance[keys[i]!]!]),
      );
    }
  }

  create2DRenderPipeline(
    device: GPUDevice,
    label: string,
    bgLayouts: GPUBindGroupLayout[],
    code: string,
    presentationFormat: GPUTextureFormat,
  ) {
    return device.createRenderPipeline({
      label: `${label}.pipeline`,
      layout: device.createPipelineLayout({
        bindGroupLayouts: bgLayouts,
      }),
      vertex: {
        module: device.createShaderModule({
          code: fullscreenTexturedQuad,
        }),
        entryPoint: 'vert_main',
      },
      fragment: {
        module: device.createShaderModule({
          code: code,
        }),
        entryPoint: 'frag_main',
        targets: [
          {
            format: presentationFormat,
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
        cullMode: 'none',
      },
    });
  }
}
