// import type { ViewProps } from 'ViewPropTypes';
import type { HostComponent, ViewProps } from 'react-native';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export interface WebgpuNativeComponentProps extends ViewProps {}

export default codegenNativeComponent<WebgpuNativeComponentProps>(
  'WGPUWebGPUView'
) as HostComponent<WebgpuNativeComponentProps>;
