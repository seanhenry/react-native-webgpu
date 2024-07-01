import type { ImageSourcePropType } from 'react-native';

interface WGPUContext {
  configure(options: GPUCanvasConfiguration): void;
  surfaceCapabilities: SurfaceCapabilities;
  getCurrentTexture(): GPUTexture | null;
  presentSurface(): void;
  width: number;
  height: number;
}

interface SurfaceCapabilities {
  formats: GPUTextureFormat[];
  alphaModes: GPUCanvasAlphaMode[];
}

interface ImageBitmap {
  width: number;
  height: number;
  close(): void;
}

declare global {
  var webGPU: {
    navigator: NavigatorGPU;
    /**
     * Pass information about the context.
     *
     * @param props Pass `identifier` from `onInit` when using multiple instances at once.
     */
    getContext(props?: {identifier: string}): WGPUContext;
    createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>
  };
}

export {}
