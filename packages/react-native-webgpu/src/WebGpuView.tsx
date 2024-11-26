import type { ImageBitmap, SurfaceBackedWebGPU, WGPUContext } from './types';
import {
  type ImageSourcePropType,
  type NativeSyntheticEvent,
  View,
  type ViewProps,
} from 'react-native';
import { useCallback, useEffect, useRef } from 'react';
import { WGPUWebGPUView } from './native';
import { styles } from './styles';
import { Backends, type OnCreateSurfaceEvent } from './specs';

export const defaultBackends = { current: Backends.All };

export interface OnCreateSurfacePayload extends SurfaceBackedWebGPU {
  createImageBitmap(source: ImageSourcePropType): Promise<ImageBitmap>;
}

export interface WebGpuViewProps extends ViewProps {
  /**
   * Android only, this is a no-op on iOS.
   *
   * Default is `Backends.All`. Choose which backends to allow for this surface.
   *
   * Note that this is only used once during surface initialisation.
   *
   * ### Android emulator
   *
   * The Android emulator doesn't support hardware acceleration for Vulkan so it will crash when attempting to use the Vulkan backend.
   * To work around it, you can set the `backends` prop to GL when using the emulator.
   *
   * ```
   * <WebGpuView backends={Platform.OS === android && isEmulator ? Backends.GL : Backends.All} />
   * ```
   *
   * Please note, it's not safe to assume that the GL backend will be identical to Vulkan.
   * Be sure to test fully on all backends used in production.
   */
  backends?: number;
  /**
   * iOS only, this is a no-op on Android.
   *
   * Set this to true if you want to smoothly animate the size of this view.
   * This will ensure that the `context` will provide the live `width` and `height`.
   * Defaults to false because this may have performance implications.
   *
   * On Android, this is automatic and doesn't require this flag.
   */
  pollSize?: boolean;
  /**
   * This callback is called once when the surface is first created and has a non-zero size.
   * Use this as the entry point for your WebGPU app.
   * @param payload Contains the WebGPU api, linked to this view.
   */
  onCreateSurface(
    payload: OnCreateSurfacePayload
  ): Promise<() => void> | (() => void) | Promise<void> | void;
  /**
   * A callback used to report an error is one occurs during initialization.
   * @param error
   */
  onError?(error: Error): void;
}

export const WebGpuView = ({
  backends = defaultBackends.current,
  pollSize = false,
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
    ({ nativeEvent }: NativeSyntheticEvent<OnCreateSurfaceEvent>) => {
      tearDown();
      if (nativeEvent?.uuid) {
        const webGPU = reactNativeWebGPU.getSurfaceBackedWebGPU(
          nativeEvent.uuid
        );
        contextRef.current = webGPU.context;
        const onDelete = onCreateSurface({
          ...webGPU,
          createImageBitmap: reactNativeWebGPU.createImageBitmap,
        });
        if (typeof onDelete === 'function') {
          onDeleteSurfaceRef.current = onDelete;
        } else if (onDelete instanceof Promise) {
          onDelete
            .then((fn) => {
              onDeleteSurfaceRef.current = fn ?? null;
            })
            .catch(rethrowError);
        }
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
        backends={backends}
        pollSize={pollSize}
        onCreateSurface={onCreateSurfaceInternal}
        style={styles.fill}
      />
    </View>
  );
};

const rethrowError = (error: unknown) => {
  setTimeout(() => {
    // Throwing error outside promise to get red screen
    throw error;
  }, 0);
};
