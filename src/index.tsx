import './constants';
import { type OnCreateSurfaceEvent, WGPUWebGPUView } from './native';
import React, { useCallback, useEffect, useRef } from 'react';
import type { ImageSourcePropType, ViewProps } from 'react-native';
import type { ImageBitmap, SurfaceBackedWebGPU, WGPUContext } from '../types/types';

export * from '../types/types'

interface OnCreateSurfacePayload extends SurfaceBackedWebGPU {
  createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>
}

export interface WebGpuViewProps extends ViewProps {
  onCreateSurface(payload: OnCreateSurfacePayload): void;
  onError?(error: Error): void;
}

export const WebGpuView = ({ onError, onCreateSurface, ...props }: WebGpuViewProps) => {
  const contextRef = useRef<WGPUContext | null>(null);
  const onErrorRef = useRef(onError);
  onErrorRef.current = onError;

  const tearDown = useCallback(() => {
    if (contextRef.current) {
      contextRef.current.unconfigure();
      contextRef.current = null;
    }
  }, [])

  const onCreateSurfaceInternal = useCallback(({ nativeEvent }: OnCreateSurfaceEvent) => {
    tearDown();
    if ('uuid' in nativeEvent) {
      const webGPU = reactNativeWebGPU.getSurfaceBackedWebGPU(nativeEvent.uuid);
      contextRef.current = webGPU.context;
      onCreateSurface({ ...webGPU, createImageBitmap: reactNativeWebGPU.createImageBitmap });
    } else {
      onErrorRef.current ? onErrorRef.current(new Error(nativeEvent.error)) : console.error(nativeEvent.error);
    }
  }, [tearDown, onCreateSurface]);

  useEffect(() => {
    return () => tearDown()
  }, [tearDown]);

  return (
    <>
      <WGPUWebGPUView {...props} onCreateSurface={onCreateSurfaceInternal} />
    </>
  );
};
