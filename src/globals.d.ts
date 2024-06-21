interface WGPUContext {
  configure(options: GPUCanvasConfiguration): void;
  surfaceCapabilities: SurfaceCapabilities;
  getCurrentTexture(): GPUTexture | null;
  presentSurface(): void;
}

interface SurfaceCapabilities {
  formats: GPUTextureFormat[];
  alphaModes: GPUCanvasAlphaMode[];
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
  };
}

export {}
