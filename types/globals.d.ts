import type { ImageSourcePropType } from 'react-native';
import type { WGPUContext, ImageBitmap, WGPUTimer } from './types';

declare global {
  var webGPU: {
    navigator: NavigatorGPU;
    /**
     * Pass information about the context.
     *
     * @param props Pass `identifier` from `onInit` when using multiple instances at once.
     */
    getContext(props?: {identifier: string}): WGPUContext;
    createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>;
    makeTimer(): WGPUTimer;
  };
}
