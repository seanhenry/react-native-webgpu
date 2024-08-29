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
