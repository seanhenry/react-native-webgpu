import Animated, {
  clamp,
  runOnJS,
  runOnUI,
  useAnimatedProps,
  useAnimatedStyle,
  useSharedValue,
} from 'react-native-reanimated';
import {LayoutChangeEvent, StyleSheet, TextInput, View} from 'react-native';
import {Gesture, GestureDetector} from 'react-native-gesture-handler';

const AnimatedTextInput = Animated.createAnimatedComponent(TextInput);

type SliderProps = {
  min: number;
  max: number;
  step: number;
  initialValue: number;
  onChange: (value: number) => void;
};

export const Slider = ({
  min,
  max,
  step,
  initialValue,
  onChange,
}: SliderProps) => {
  const normalizedValue = useSharedValue(
    normalizedValueBetween(initialValue, min, max),
  );
  const sliderWidth = useSharedValue(0);
  const lastValue = useSharedValue(0);
  const props = useSharedValue({min, max, step});
  const pan = Gesture.Pan()
    .onStart(() => {
      lastValue.value = normalizedValue.value;
    })
    .onUpdate(event => {
      const newValue = clamp(
        lastValue.value + event.translationX / sliderWidth.value,
        0,
        1,
      );
      if (newValue === normalizedValue.value) {
        return;
      }
      normalizedValue.value = newValue;
      runOnJS(onChange)(
        unnormalizedValueBetween(
          normalizedValue.value,
          props.value.min,
          props.value.max,
          props.value.step,
        ),
      );
    });

  const animatedStyle = useAnimatedStyle(() => ({
    width: `${normalizedValue.value * 100}%`,
  }));

  const animatedProps = useAnimatedProps(() => ({
    text: `${unnormalizedValueBetween(
      normalizedValue.value,
      props.value.min,
      props.value.max,
      props.value.step,
    )}`,
  }));

  const onLayout = ({nativeEvent}: LayoutChangeEvent) => {
    runOnUI((width: number) => {
      sliderWidth.value = width;
    })(nativeEvent.layout.width);
  };

  return (
    <View style={styles.container}>
      <GestureDetector gesture={pan}>
        <View style={styles.sliderBackground} onLayout={onLayout}>
          <Animated.View style={[styles.sliderForeground, animatedStyle]} />
        </View>
      </GestureDetector>
      <View style={styles.textContainer}>
        <AnimatedTextInput
          // @ts-expect-error text is not a prop type
          animatedProps={animatedProps}
          style={styles.text}
          editable={false}
          value={`${unnormalizedValueBetween(
            normalizedValue.value,
            min,
            max,
            step,
          )}`}
        />
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flexDirection: 'row',
    gap: 4,
  },
  sliderBackground: {
    backgroundColor: '#303030',
    height: 30,
    width: 100,
  },
  sliderForeground: {
    backgroundColor: '#2FA1D6',
    height: '100%',
  },
  textContainer: {
    backgroundColor: '#303030',
    justifyContent: 'center',
    paddingHorizontal: 4,
  },
  text: {
    color: '#2FA1D6',
  },
});

const normalizedValueBetween = (value: number, min: number, max: number) => {
  if (max === min) {
    return min;
  }
  return (value - min) / (max - min);
};

const unnormalizedValueBetween = (
  value: number,
  min: number,
  max: number,
  step: number,
) => {
  'worklet';
  const unnormalized = value * (max - min);
  if (step === 0) {
    step = 1;
  }
  const stepped = step * Math.round(unnormalized / step);
  return clamp(min + stepped, min, max);
};
