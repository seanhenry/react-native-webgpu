import type {
  WebGpuViewProps,
  OnCreateSurfacePayload,
} from "react-native-webgpu";
import type { FC } from "react";

export declare const enableDebugLogging: (enable: boolean) => void;

interface ThreeOnCreateSurfacePayload extends OnCreateSurfacePayload {
  adapter: GPUAdapter;
  device: GPUDevice;
  rendererParameters: object;
}

interface ThreeWebGpuViewProps
  extends Omit<WebGpuViewProps, "onCreateSurface"> {
  onCreateSurface(payload: ThreeOnCreateSurfacePayload): void;
}

export declare const ThreeWebGpuView: FC<ThreeWebGpuViewProps>;
