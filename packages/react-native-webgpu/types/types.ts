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

export interface VideoPlayer {
  nextPixelBuffer(): PixelBuffer | null;
  seek(position: number): void;
  play(): void;
  release(): void;
}

export interface PixelBuffer {
  planeCount: number;
  isInterleaved: boolean;
  getWidthOfPlane(index: number): number;
  getHeightOfPlane(index: number): number;
  getArrayBufferOfPlane(index: number): ArrayBuffer;
  getBytesPerRowOfPlane(index: number): number;
  width: number;
  height: number;
  release(): void;
}

export interface CopyExternalImageToTextureCompatible {
  __isCopyExternalImageToTextureCompatible: true;
  arrayBuffer: ArrayBuffer;
  width: number;
  height: number;
}

export interface WGPUExperimental {
  inflate(arrayBuffer: ArrayBuffer): ArrayBuffer;
  makeVideoPlayer(url: string): VideoPlayer;
}
