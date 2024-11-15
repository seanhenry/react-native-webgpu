import type {
  ImageResolvedAssetSource,
  ImageSourcePropType,
} from 'react-native';

export interface SurfaceBackedWebGPU {
  navigator: NavigatorGPU;
  requestAnimationFrame(callback: (time: number) => void): void;
  context: WGPUContext;
}

export interface HeadlessWebGPU {
  navigator: {
    gpu: Pick<GPU, 'requestAdapter'>;
  };
}

export interface WGPUContext
  extends Omit<GPUCanvasContext, 'getCurrentTexture'> {
  surfaceCapabilities(adapter: GPUAdapter): WGPUSurfaceCapabilities;
  getCurrentTexture(): GPUTexture | null;
  presentSurface(): void;
  width: number;
  height: number;
  pointWidth: number;
  pointHeight: number;
  scale: number;
}

export interface WGPUSurfaceCapabilities {
  formats: GPUTextureFormat[];
  alphaModes: GPUCanvasAlphaMode[];
  usages: GPUTextureUsageFlags;
}

export interface ImageBitmap {
  width: number;
  height: number;
  close(): void;
}

export interface CopyExternalImageToTextureCompatible {
  __isCopyExternalImageToTextureCompatible: true;
  arrayBuffer: ArrayBuffer;
  width: number;
  height: number;
}

declare global {
  var __reactNativeWebGPU: {
    createImageBitmap(source: ImageResolvedAssetSource): Promise<ImageBitmap>;
    getSurfaceBackedWebGPU(uuid: string): SurfaceBackedWebGPU;
    getHeadlessWebGPU(uuid: string): HeadlessWebGPU;
  };

  var reactNativeWebGPU: {
    createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>;
    getSurfaceBackedWebGPU(uuid: string): SurfaceBackedWebGPU;
    getHeadlessWebGPU(uuid: string): HeadlessWebGPU;
  };
}
