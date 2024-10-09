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
  Platform.select({ ios: "- You have run 'pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo Go\n';

export type OnCreateSurfaceEvent = NativeSyntheticEvent<
  { uuid: string } | { error: string }
>;
export type WGPUWebGPUViewProps = ViewProps & {
  onCreateSurface(event: OnCreateSurfaceEvent): void;
};

export const WGPUWebGPUView =
  requireNativeComponent<WGPUWebGPUViewProps>('WGPUWebGPUView');
const webGpuJsi = NativeModules.WGPUJsi;

if (!webGpuJsi) {
  throw new Error(LINKING_ERROR);
}

if (!WGPUWebGPUView) {
  throw new Error(LINKING_ERROR);
}

function createImageBitmap(source: ImageSourcePropType) {
  const resolvedSource = Image.resolveAssetSource(source);
  return __reactNativeWebGPU.createImageBitmap(resolvedSource);
}

globalThis.reactNativeWebGPU = new Proxy(__reactNativeWebGPU, {
  get: function (target, prop, receiver) {
    if (prop === 'createImageBitmap') {
      return createImageBitmap;
    }
    return Reflect.get(target, prop, receiver);
  },
});
