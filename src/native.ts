import {
  Image,
  type ImageSourcePropType,
  NativeModules,
  type NativeSyntheticEvent,
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
  onInit(props: NativeSyntheticEvent<{identifier: string}>): void,
  /**
   * Used to identify the view with the graphics context.
   */
  identifier: string
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

const globalObj = (globalThis as any).__webGPUNative;

function createImageBitmap(source: ImageSourcePropType) {
  const resolvedSource = Image.resolveAssetSource(source)
  return globalObj.__createImageBitmap(resolvedSource);
}

globalThis.webGPU = new Proxy(globalObj, {
  get: function(target, prop, receiver) {
    if (prop === 'createImageBitmap') {
      return createImageBitmap;
    }
    return Reflect.get(target, prop, receiver);
  }
});
