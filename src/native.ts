import {
  NativeModules,
  Platform,
  requireNativeComponent,
  type ViewProps,
} from 'react-native';

const LINKING_ERROR =
  `The package 'react-native-webgpu' doesn't seem to be linked. Make sure: \n\n` +
  Platform.select({ ios: '- You have run \'pod install\'\n', default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo Go\n';

export type WGPUWebGPUViewProps = ViewProps & {
  /**
   * Called on the native UI thread to configure the device and pipeline
   */
  onInit: (props: {identifier: string}) => void,
  /**
   * Used to identify the view with the graphics context. `main` will be used by default
   */
  identifier?: string
}

export const WGPUWebGPUView = requireNativeComponent<WGPUWebGPUViewProps>('WGPUWebGPUView')
const webGpuJsi = NativeModules.WGPUJsi

if (!webGpuJsi) {
  throw new Error(LINKING_ERROR)
}

if (!WGPUWebGPUView) {
  throw new Error(LINKING_ERROR)
}

if (!webGpuJsi.install()) {
  throw new Error("Failed to install JSI");
}
