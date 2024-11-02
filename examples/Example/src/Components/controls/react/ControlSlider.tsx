import Animated, {
  runOnJS,
  runOnUI,
  useAnimatedStyle,
  useSharedValue,
} from 'react-native-reanimated';
import {LayoutChangeEvent, StyleSheet, TextInput, View} from 'react-native';
import {Gesture, GestureDetector} from 'react-native-gesture-handler';
import {Observable} from '../../../utils/observable';
import {useEffect, useState} from 'react';
import {ControlBackground} from './ControlBackground';
import {ControlLabel} from './ControlLabel';
import {ControlInput} from './ControlInput';
import {clamp, lerpWithStep, normalizedBetween} from '../sliderUtils';
import {controlBackgroundColor, controlForegroundColor} from './controlsStyles';

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
  const [value, setValue] = useState(`${initialValue}`);
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
      const steppedValue = lerpWithStep(
        normalizedValue.value,
        props.value.min,
        props.value.max,
        props.value.step,
      );
      runOnJS(onChange)(steppedValue);
      runOnJS(setValue)(`${steppedValue}`);
    })
    .onEnd(() => {
      runOnJS(onFinishChange)();
    });

  const animatedStyle = useAnimatedStyle(() => ({
    width: `${clamp(normalizedValue.value, 0, 1) * 100}%`,
  }));

  const onLayout = ({nativeEvent}: LayoutChangeEvent) => {
    runOnUI((width: number) => {
      sliderWidth.value = width;
    })(nativeEvent.layout.width);
  };

  const onChangeText = (text: string) => {
    setValue(text);
    const newValue = parseFloat(text);
    if (!isNaN(newValue)) {
      onChange(newValue);
      onFinishChange();
      observable.post(newValue);
    }
  };

  useEffect(() => {
    return observable.add(newValue => {
      setValue(`${newValue}`);
      runOnUI((v: number) => {
        normalizedValue.value = normalizedBetween(
          v,
          props.value.min,
          props.value.max,
        );
      })(newValue);
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
          <TextInput
            style={styles.text}
            editable={!disabled}
            value={value}
            onChangeText={onChangeText}
            defaultValue={`${initialValue}`}
            keyboardType="numeric"
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
