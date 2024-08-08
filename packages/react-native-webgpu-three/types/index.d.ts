import { WebGpuViewProps } from 'react-native-webgpu';
import { OnCreateSurfacePayload } from 'react-native-webgpu/src';
import React from 'react';

export declare const enableDebugLogging: (enable: boolean) => void;

interface ThreeOnCreateSurfacePayload extends OnCreateSurfacePayload {
  adapter: GPUAdapter;
  device: GPUDevice;
  rendererParameters: object
}

interface ThreeWebGpuViewProps extends Omit<WebGpuViewProps, 'onCreateSurface'> {
  onCreateSurface(payload: ThreeOnCreateSurfacePayload): void;
}

export declare const ThreeWebGpuView: React.FC<ThreeWebGpuViewProps>;
