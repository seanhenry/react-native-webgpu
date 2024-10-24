import {useCallback, useRef, useState} from 'react';
import {HudText} from './HudText';

type SetText = (text: string) => void;

export const useHudText = () => {
  const setTextRef = useRef<SetText | null>(null);
  const Component = () => {
    // Using state because `setNativeProps` doesn't trigger a layout
    const [text, setText] = useState('');
    setTextRef.current = setText;
    return <HudText value={text} />;
  };
  Component.displayName = 'HudText';
  const setText = useCallback((text: string) => {
    setTextRef.current?.(text);
  }, []);
  return {
    setText,
    HudText: Component,
  };
};
