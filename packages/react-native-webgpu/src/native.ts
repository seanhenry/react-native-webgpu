import { Image, type ImageSourcePropType, Platform } from 'react-native';
import { NativeWebgpuModule, WebgpuNativeComponent } from './specs';

const LINKING_ERROR =
  `The package 'react-native-webgpu' doesn't seem to be linked. Make sure: \n\n` +
  Platform.select({ ios: "- You have run 'pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo Go\n';

export const WGPUWebGPUView = WebgpuNativeComponent;

if (!NativeWebgpuModule) {
  throw new Error(LINKING_ERROR);
}

if (!WGPUWebGPUView) {
  throw new Error(LINKING_ERROR);
}

function createImageBitmap(source: ImageSourcePropType) {
  const resolvedSource = Image.resolveAssetSource(source);
  return __reactNativeWebGPU.createImageBitmap(resolvedSource);
}

export const installWithThreadId = (threadId: string) => {
  if (!NativeWebgpuModule.installWithThreadId(threadId)) {
    throw new Error('Failed to install JSI');
  }
  globalThis.reactNativeWebGPU = new Proxy(__reactNativeWebGPU, {
    get: function (target, prop, receiver) {
      if (prop === 'createImageBitmap') {
        return createImageBitmap;
      }
      return Reflect.get(target, prop, receiver);
    },
  });
};

export const install = () => installWithThreadId('__js');
