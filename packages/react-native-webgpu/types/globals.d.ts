import type {
  ImageResolvedAssetSource,
  ImageSourcePropType,
} from 'react-native';
import type {
  HeadlessWebGPU,
  ImageBitmap,
  SurfaceBackedWebGPU,
  WGPUExperimental,
} from './types';

declare global {
  var __reactNativeWebGPU: {
    createImageBitmap(source: ImageResolvedAssetSource): Promise<ImageBitmap>;
    getSurfaceBackedWebGPU(uuid: string): SurfaceBackedWebGPU;
    getHeadlessWebGPU(uuid: string): HeadlessWebGPU;
    experimental: WGPUExperimental;
  };

  var reactNativeWebGPU: {
    createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>;
    getSurfaceBackedWebGPU(uuid: string): SurfaceBackedWebGPU;
    getHeadlessWebGPU(uuid: string): HeadlessWebGPU;
    experimental: WGPUExperimental;
  };

  var reactNativeWebGPUThreads: {
    spawn(input: ThreadInput): void;
    attachSurface(input: AttachSurfaceInput): void;
  };

  var reactNativeWebGPUThreadsInstance: {
    onAttachSurface(callback: (payload: OnAttachSurfacePayload) => void): void;
    getContext(): ThreadInstanceContext;
  };
}

type ThreadInput = {
  bundleId: string;
  threadId: string;
};

type AttachSurfaceInput = {
  uuid: string;
  threadId: string;
};

type OnAttachSurfacePayload = {
  uuid: string;
};

type ThreadInstanceContext = {
  threadId: string;
};
