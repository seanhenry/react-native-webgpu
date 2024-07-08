import './constants';
import { WGPUWebGPUView } from './native';
import React, { useCallback, useEffect, useRef } from 'react';
import type { NativeSyntheticEvent, ViewProps } from 'react-native';
import type { WGPUContext, WGPUTimer } from '../types/types';
export * from '../types/types'

export interface WebGpuViewProps extends ViewProps {
  onInit(props: { context: WGPUContext, timer: WGPUTimer }): void;

  onError?(error: Error): void;

  /**
   * Used to identify the view with the graphics context.
   */
  identifier: string;
}

export const WebGpuView = ({ identifier, onInit, onError, ...props }: WebGpuViewProps) => {
  const contextRef = useRef<WGPUContext | null>(null);
  const timerRef = useRef<WGPUTimer | null>(null);
  const onErrorRef = useRef(onError);
  onErrorRef.current = onError;

  const tearDown = useCallback(() => {
    if (timerRef.current) {
      timerRef.current.invalidate();
    }
    if (contextRef.current) {
      contextRef.current.destroy();
      contextRef.current = null;
    }
  }, [])

  const onInitInternal = useCallback(({ nativeEvent }: NativeSyntheticEvent<{ identifier: string }>) => {
    tearDown();
    try {
      const wgpuContext = webGPU.getContext(nativeEvent);
      contextRef.current = wgpuContext;
      const timer = webGPU.makeTimer();
      timer.start();
      timerRef.current = timer;
      onInit({ context: wgpuContext, timer });
    } catch (error) {
      onErrorRef.current ? onErrorRef.current(error as Error) : console.error(error);
    }
  }, [tearDown, onInit]);

  useEffect(() => {
    return () => tearDown()
  }, [tearDown]);

  return (
    <WGPUWebGPUView {...props} identifier={identifier} onInit={onInitInternal} />
  );
};
