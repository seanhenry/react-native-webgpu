import {WebGpuView} from 'react-native-webgpu';
import {useWindowDimensions} from 'react-native';
import {useEffect, useState} from 'react';
import {runResizeCanvas} from './runResizeCanvas';

export function ResizeCanvas() {
  const window = useWindowDimensions();
  const [height, setHeight] = useState(window.height * 0.2);

  useEffect(() => {
    let count = 1;
    const interval = setInterval(() => {
      const heightPercentage = 0.2 * (1 + (count % 4));
      setHeight(heightPercentage * window.height);
      count += 1;
    }, 500);
    return () => clearInterval(interval);
  }, [window.height]);

  return <WebGpuView onCreateSurface={runResizeCanvas} style={{height}} />;
}
