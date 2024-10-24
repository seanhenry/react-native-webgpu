import {useCallback, useRef} from 'react';
import {TextInput} from 'react-native';
import {HudText} from './HudText';

export const useHudText = () => {
  const textRef = useRef<TextInput | null>(null);
  const Component = () => {
    return <HudText ref={textRef} />;
  };
  Component.displayName = 'HudText';
  const setText = useCallback((text: string) => {
    textRef.current?.setNativeProps({text});
  }, []);
  return {
    setText,
    HudText: Component,
  };
};
