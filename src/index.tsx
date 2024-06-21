import './constants'
import { WGPUWebGPUView, type WGPUWebGPUViewProps } from './native';
import React, { useCallback } from 'react';

export const WebGpuView = ({identifier, onInit, ...props}: WGPUWebGPUViewProps) => {
  const onInitInternal = useCallback(({nativeEvent}: any) => onInit(nativeEvent), [onInit])
  return (
    <WGPUWebGPUView {...props} identifier={identifier ?? 'main'} onInit={onInitInternal} />
  )
}
