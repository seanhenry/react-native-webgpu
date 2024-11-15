declare global {
  var reactNativeWebGPUExperimental: {
    inflate(arrayBuffer: ArrayBuffer): ArrayBuffer;
    makeVideoPlayer(url: string): VideoPlayer;
  };

  var reactNativeWebGPUThreads: {
    spawn(input: ThreadInput): void;
    attachSurface(input: AttachSurfaceInput): void;
  };

  var reactNativeWebGPUThreadsInstance: {
    onAttachSurface?: OnAttachSurfacePayload | null;
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
