import { Platform } from 'react-native';
import { NativeWebgpuExperimentalModule } from './specs';
import { WebgpuNativeComponent } from 'react-native-webgpu';

const LINKING_ERROR =
  `The package 'react-native-webgpu-experimental' doesn't seem to be linked. Make sure: \n\n` +
  Platform.select({ ios: "- You have run 'pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo Go\n';

if (!NativeWebgpuExperimentalModule) {
  throw new Error(LINKING_ERROR);
}

export const WGPUWebGPUView = WebgpuNativeComponent;

if (!WGPUWebGPUView) {
  throw new Error(LINKING_ERROR);
}

export const installWithThreadId = (threadId: string) => {
  if (!NativeWebgpuExperimentalModule.installWithThreadId(threadId)) {
    throw new Error('Failed to install JSI');
  }
};

export const install = () => installWithThreadId('__js');

export const { ENABLE_THREADS } = NativeWebgpuExperimentalModule.getConstants();
