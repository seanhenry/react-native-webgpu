import { type NativeSyntheticEvent, View, type ViewProps } from 'react-native';
import { useCallback, useEffect, useRef } from 'react';
import type { WGPUContext } from '../types/types';
import { ENABLE_THREADS, WGPUWebGPUView } from './native';
import { styles } from './styles';
import type { OnCreateSurfaceEvent } from './specs/WebgpuNativeComponent';

export interface ThreadWebGpuViewProps extends ViewProps {
  threadId: string;
  onError?(error: Error): void;
}

export const ThreadWebGpuView = ({
  onError,
  threadId,
  ...props
}: ThreadWebGpuViewProps) => {
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
    ({ nativeEvent }: NativeSyntheticEvent<OnCreateSurfaceEvent>) => {
      tearDown();
      if (nativeEvent?.uuid) {
        reactNativeWebGPUThreads.attachSurface({
          uuid: nativeEvent.uuid,
          threadId,
        });
        const webGPU = reactNativeWebGPU.getSurfaceBackedWebGPU(
          nativeEvent.uuid
        );
        contextRef.current = webGPU.context;
      } else {
        onErrorRef.current
          ? onErrorRef.current(new Error(nativeEvent.error))
          : console.error(nativeEvent.error);
      }
    },
    [tearDown, threadId]
  );

  useEffect(() => {
    return () => tearDown();
  }, [tearDown]);

  useEffect(() => {
    if (!ENABLE_THREADS) {
      console.error(
        'Using ThreadWebGpuView but WGPU_ENABLE_THREADS feature is off'
      );
    }
  }, []);

  if (!ENABLE_THREADS) {
    return null;
  }

  return (
    <View {...props}>
      <WGPUWebGPUView
        onCreateSurface={onCreateSurfaceInternal}
        style={styles.fill}
      />
    </View>
  );
};
