import './constants';
import { type OnCreateSurfaceEvent, WGPUWebGPUView } from './native';
import { useCallback, useEffect, useRef } from 'react';
import {
  type ImageSourcePropType,
  StyleSheet,
  View,
  type ViewProps,
} from 'react-native';
import type {
  ImageBitmap,
  SurfaceBackedWebGPU,
  WGPUContext,
} from '../types/types';

export * from '../types/types';

export interface OnCreateSurfacePayload extends SurfaceBackedWebGPU {
  createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>;
}

export interface WebGpuViewProps extends ViewProps {
  onCreateSurface(
    payload: OnCreateSurfacePayload
  ): Promise<() => void> | (() => void) | Promise<void> | void;
  onError?(error: Error): void;
}

export const WebGpuView = ({
  onError,
  onCreateSurface,
  ...props
}: WebGpuViewProps) => {
  const contextRef = useRef<WGPUContext | null>(null);
  const onDeleteSurfaceRef = useRef<(() => void) | null>(null);
  const onErrorRef = useRef(onError);
  onErrorRef.current = onError;

  const tearDown = useCallback(() => {
    if (typeof onDeleteSurfaceRef.current === 'function') {
      onDeleteSurfaceRef.current();
      onDeleteSurfaceRef.current = null;
    }
    if (contextRef.current) {
      contextRef.current.unconfigure();
      contextRef.current = null;
    }
  }, []);

  const onCreateSurfaceInternal = useCallback(
    ({ nativeEvent }: OnCreateSurfaceEvent) => {
      tearDown();
      if ('uuid' in nativeEvent) {
        const webGPU = reactNativeWebGPU.getSurfaceBackedWebGPU(
          nativeEvent.uuid
        );
        contextRef.current = webGPU.context;
        Promise.resolve(
          onCreateSurface({
            ...webGPU,
            createImageBitmap: reactNativeWebGPU.createImageBitmap,
          })
        )
          .then((fn) => {
            onDeleteSurfaceRef.current = fn ?? null;
          })
          .catch(rethrowError);
      } else {
        onErrorRef.current
          ? onErrorRef.current(new Error(nativeEvent.error))
          : console.error(nativeEvent.error);
      }
    },
    [tearDown, onCreateSurface]
  );

  useEffect(() => {
    return () => tearDown();
  }, [tearDown]);

  return (
    <View {...props}>
      <WGPUWebGPUView
        onCreateSurface={onCreateSurfaceInternal}
        style={styles.fill}
      />
    </View>
  );
};

const styles = StyleSheet.create({
  fill: {
    width: '100%',
    height: '100%',
  },
});

const rethrowError = (error: unknown) => {
  setTimeout(() => {
    // Throwing error outside promise to get red screen
    throw error;
  }, 0);
};
