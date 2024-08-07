import type { ImageResolvedAssetSource, ImageSourcePropType } from 'react-native';
import type { HeadlessWebGPU, ImageBitmap, SurfaceBackedWebGPU } from './types';

declare global {
  var __reactNativeWebGPU: {
    createImageBitmap(source: ImageResolvedAssetSource): Promise<ImageBitmap>;
    getSurfaceBackedWebGPU(uuid: string): SurfaceBackedWebGPU;
    getHeadlessWebGPU(uuid: string): HeadlessWebGPU;
  };

  var reactNativeWebGPU: {
    createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>;
    getSurfaceBackedWebGPU(uuid: string): SurfaceBackedWebGPU;
    getHeadlessWebGPU(uuid: string): HeadlessWebGPU;
  };
}
