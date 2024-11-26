/* eslint-disable no-bitwise */
import type { HostComponent, ViewProps } from 'react-native';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';
import type {
  DirectEventHandler,
  Int32,
} from 'react-native/Libraries/Types/CodegenTypes';

export enum Backends {
  All = 0x00000000,
  Vulkan = 1 << 0,
  GL = 1 << 1,
  Metal = 1 << 2,
  DX12 = 1 << 3,
  DX11 = 1 << 4,
  BrowserWebGPU = 1 << 5,
  Primary = Backends.Vulkan |
    Backends.Metal |
    Backends.DX12 |
    Backends.BrowserWebGPU,
  Secondary = Backends.GL | Backends.DX11,
}

export type OnCreateSurfaceEvent = {
  uuid?: string;
  error?: string;
};

export interface WebgpuNativeComponentProps extends ViewProps {
  pollSize: boolean;
  backends: Int32;
  onCreateSurface: DirectEventHandler<OnCreateSurfaceEvent>;
}

export default codegenNativeComponent<WebgpuNativeComponentProps>(
  'WGPUWebGPUView'
) as HostComponent<WebgpuNativeComponentProps>;
