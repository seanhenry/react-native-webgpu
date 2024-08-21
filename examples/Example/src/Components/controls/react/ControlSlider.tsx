import Animated, {
  runOnJS,
  runOnUI,
  useAnimatedProps,
  useAnimatedStyle,
  useSharedValue,
} from 'react-native-reanimated';
import {LayoutChangeEvent, StyleSheet, TextInput, View} from 'react-native';
import {Gesture, GestureDetector} from 'react-native-gesture-handler';
import {Observable} from '../../../utils/observable';
import {useEffect} from 'react';
import {ControlBackground} from './ControlBackground';
import {ControlLabel} from './ControlLabel';
import {ControlInput} from './ControlInput';
import {
  clamp,
  formatSteppedNumber,
  lerpWithStep,
  normalizedBetween,
} from '../sliderUtils.ts';
import {
  controlBackgroundColor,
  controlForegroundColor,
} from './controlsStyles.ts';

const AnimatedTextInput = Animated.createAnimatedComponent(TextInput);

type ControlSliderProps = {
  disabled: boolean;
  min: number;
  max: number;
  step: number;
  initialValue: number;
  title: string;
  onChange: (value: number) => void;
  onFinishChange: () => void;
  observable: Observable<number>;
};

export const ControlSlider = ({
  disabled,
  min,
  max,
  step,
  initialValue,
  title,
  onChange,
  onFinishChange,
  observable,
}: ControlSliderProps) => {
  const normalizedValue = useSharedValue(
    normalizedBetween(initialValue, min, max),
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
        lerpWithStep(
          normalizedValue.value,
          props.value.min,
          props.value.max,
          props.value.step,
        ),
      );
    })
    .onEnd(() => {
      runOnJS(onFinishChange)();
    });

  const animatedStyle = useAnimatedStyle(() => ({
    width: `${clamp(normalizedValue.value, 0, 1) * 100}%`,
  }));

  const animatedProps = useAnimatedProps(() => {
    return {
      text: formatSteppedNumber(
        lerpWithStep(
          normalizedValue.value,
          props.value.min,
          props.value.max,
          props.value.step,
        ),
        props.value.step,
      ),
    };
  });

  const onLayout = ({nativeEvent}: LayoutChangeEvent) => {
    runOnUI((width: number) => {
      sliderWidth.value = width;
    })(nativeEvent.layout.width);
  };

  useEffect(() => {
    return observable.add(value => {
      runOnUI((v: number) => {
        normalizedValue.value = normalizedBetween(
          v,
          props.value.min,
          props.value.max,
        );
      })(value);
    });
  }, [normalizedValue, observable, props]);

  return (
    <ControlBackground>
      <ControlLabel text={title} />
      <ControlInput>
        <GestureDetector gesture={pan}>
          <View
            style={styles.sliderBackground}
            onLayout={onLayout}
            pointerEvents={disabled ? 'none' : 'auto'}>
            <Animated.View style={[styles.sliderForeground, animatedStyle]} />
          </View>
        </GestureDetector>
        <View style={styles.textContainer}>
          <AnimatedTextInput
            // @ts-expect-error text is not a prop type
            animatedProps={animatedProps}
            style={styles.text}
            editable={false}
            multiline
            defaultValue={`${initialValue}`}
          />
        </View>
      </ControlInput>
    </ControlBackground>
  );
};

const styles = StyleSheet.create({
  sliderBackground: {
    backgroundColor: controlBackgroundColor,
    height: 30,
    width: 100,
  },
  sliderForeground: {
    backgroundColor: controlForegroundColor,
    height: '100%',
  },
  textContainer: {
    backgroundColor: controlBackgroundColor,
    justifyContent: 'center',
    paddingHorizontal: 4,
  },
  text: {
    color: controlForegroundColor,
    padding: 0,
    margin: 0,
  },
});
