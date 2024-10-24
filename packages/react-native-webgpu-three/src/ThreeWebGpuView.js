import {WebGpuView} from 'react-native-webgpu';
import {useCallback, useRef} from 'react';
import {GPUTextureFormat} from 'three/examples/jsm/renderers/webgpu/utils/WebGPUConstants';
import {TouchEventsAdapter} from './TouchEventsAdapter';
import {makeCanvasProxy} from './CanvasProxy';

export const ThreeWebGpuView = ({onCreateSurface, ...props}) => {
  const touchEventsRef = useRef({
    onTouchStart: () => {},
    onTouchMove: () => {},
    onTouchEnd: () => {},
    onTouchCancel: () => {},
  });

  const threeOnCreateSurface = useCallback(
    async payload => {
      const adapter = await payload.navigator.gpu.requestAdapter();
      const supportedFeatures = [];
      for (const name of adapter.features) {
        if (adapter.features.has(name)) {
          supportedFeatures.push(name);
        }
      }
      const device = await adapter.requestDevice({
        requiredFeatures: supportedFeatures,
      });
      const eventsAdapter = new TouchEventsAdapter(touchEventsRef);
      const canvas = makeCanvasProxy({eventsAdapter, payload});

      global.createImageBitmap = __reactNativeWebGPU.createImageBitmap;
      GPUTextureFormat.BGRA8Unorm =
        payload.navigator.gpu.getPreferredCanvasFormat();

      return onCreateSurface({
        ...payload,
        adapter,
        device,
        eventsAdapter,
        rendererParameters: {
          context: payload.context,
          device,
          canvas,
        },
      });
    },
    [onCreateSurface],
  );

  return (
    <WebGpuView
      {...props}
      onCreateSurface={threeOnCreateSurface}
      onTouchStart={e => touchEventsRef.current.onTouchStart(e)}
      onTouchMove={e => touchEventsRef.current.onTouchMove(e)}
      onTouchEnd={e => touchEventsRef.current.onTouchEnd(e)}
      onTouchCancel={e => touchEventsRef.current.onTouchCancel(e)}
    />
  );
};
