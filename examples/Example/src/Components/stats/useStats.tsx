import {useRef, useState} from 'react';
import {HudText} from './HudText';

export const useStats = () => {
  const setTextRef = useRef((_text: string) => {});
  const statsRef = useRef({
    frameDurations: [] as number[],
    current: -1,
    now() {
      return performance.now();
    },
    begin() {
      this.current = this.now();
    },
    end() {
      if (this.current === -1) {
        console.warn('Stats.begin() was not called');
        return;
      }
      this.frameDurations.push(this.now() - this.current);
      this.current = -1;

      const SAMPLE_INTERVAL = 30;
      if (this.frameDurations.length === SAMPLE_INTERVAL) {
        const mean =
          this.frameDurations.reduce((acc, t) => acc + t, 0) / SAMPLE_INTERVAL;
        this.frameDurations = [];
        setTextRef.current(`FPS: ${Math.round(1000 / mean)}`);
      }
    },
  });

  const Component = () => {
    const [text, setText] = useState('FPS:');
    setTextRef.current = setText;
    return <HudText value={text} />;
  };
  Component.displayName = 'Stats';
  return {
    stats: statsRef.current,
    Stats: Component,
  };
};
