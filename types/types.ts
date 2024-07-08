

export interface WGPUContext {
  configure(options: GPUCanvasConfiguration): void;
  surfaceCapabilities: WGPUSurfaceCapabilities;
  getCurrentTexture(): GPUTexture | null;
  presentSurface(): void;
  destroy(): void;
  width: number;
  height: number;
}

export interface WGPUSurfaceCapabilities {
  formats: GPUTextureFormat[];
  alphaModes: GPUCanvasAlphaMode[];
}

export interface WGPUTimer {
  start(): void;
  stop(): void;
  invalidate(): void;
  requestAnimationFrame(callback: (time: number) => void): void;
}

export interface ImageBitmap {
  width: number;
  height: number;
  close(): void;
}
