import {WebGpuView} from 'react-native-webgpu';
import {
  Animated,
  Easing,
  StyleSheet,
  useAnimatedValue,
  useWindowDimensions,
} from 'react-native';
import {useEffect} from 'react';
import {runResizeCanvas} from '../ResizeCanvas/runResizeCanvas';

export function AnimateCanvas() {
  const window = useWindowDimensions();
  const bottom = useAnimatedValue(0);

  useEffect(() => {
    Animated.loop(
      Animated.sequence([
        Animated.timing(bottom, {
          toValue: window.height * 0.5,
          duration: 2000,
          useNativeDriver: false,
          easing: Easing.linear,
        }),
        Animated.timing(bottom, {
          toValue: 0,
          duration: 2000,
          useNativeDriver: false,
          easing: Easing.linear,
        }),
      ]),
    ).start();
  }, [bottom, window.height]);

  return (
    <Animated.View style={[styles.view, {bottom}]}>
      <WebGpuView onCreateSurface={runResizeCanvas} pollSize />
    </Animated.View>
  );
}

const styles = StyleSheet.create({
  view: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
  },
});
