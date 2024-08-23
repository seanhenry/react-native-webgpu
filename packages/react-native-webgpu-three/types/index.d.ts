import type {
  WebGpuViewProps,
  OnCreateSurfacePayload,
} from 'react-native-webgpu';
import type {FC} from 'react';
import type {EventDispatcher} from '@types/three';

export declare const enableDebugLogging: (enable: boolean) => void;

interface ThreeOnCreateSurfacePayload extends OnCreateSurfacePayload {
  adapter: GPUAdapter;
  device: GPUDevice;
  rendererParameters: object;
  eventsAdapter: EventDispatcher<{
    pointerdown: PointerEvent;
    pointermove: PointerEvent;
    pointerup: PointerEvent;
    pointercancel: PointerEvent;
  }>;
}

interface PointerEvent {
  pointerId: string;
  pointerType: 'touch';
  pageX: number;
  pageY: number;
  clientX: number;
  clientY: number;
}

interface ThreeWebGpuViewProps
  extends Omit<WebGpuViewProps, 'onCreateSurface'> {
  onCreateSurface(payload: ThreeOnCreateSurfacePayload): void;
}

export declare const ThreeWebGpuView: FC<ThreeWebGpuViewProps>;
