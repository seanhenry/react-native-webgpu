// import type { ViewProps } from 'ViewPropTypes';
import type { HostComponent, ViewProps } from 'react-native';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';
import type { DirectEventHandler } from 'react-native/Libraries/Types/CodegenTypes';

export type OnCreateSurfaceEvent = {
  uuid?: string;
  error?: string;
};

export interface WebgpuNativeComponentProps extends ViewProps {
  onCreateSurface: DirectEventHandler<OnCreateSurfaceEvent>;
}

export default codegenNativeComponent<WebgpuNativeComponentProps>(
  'WGPUWebGPUView'
) as HostComponent<WebgpuNativeComponentProps>;
