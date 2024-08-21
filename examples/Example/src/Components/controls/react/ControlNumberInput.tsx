import {Observable} from '../../../utils/observable';
import {TextInput} from 'react-native';
import {useEffect, useState} from 'react';
import {ControlBackground} from './ControlBackground';
import {ControlLabel} from './ControlLabel';
import {ControlInput} from './ControlInput';
import {controlsStyles} from './controlsStyles';
import {clamp} from '../sliderUtils';

type ControlNumberInputProps = {
  title: string;
  disabled: boolean;
  initialValue: number;
  min?: number;
  max?: number;
  observable: Observable<number>;
  onChange: (value: number) => void;
  onFinishChange: () => void;
};

export const ControlNumberInput = ({
  min,
  max,
  title,
  disabled,
  initialValue,
  observable,
  onChange,
  onFinishChange,
}: ControlNumberInputProps) => {
  const [value, setValue] = useState(`${initialValue}`);
  useEffect(() => observable.add(val => setValue(`${val}`)), [observable]);

  return (
    <ControlBackground>
      <ControlLabel text={title} />
      <ControlInput>
        <TextInput
          onTouchStart={e => e.stopPropagation()} // Prevent keyboard from being dismissed
          keyboardType="numeric"
          value={value}
          onChangeText={setValue}
          onBlur={() => {
            const num = Number.parseFloat(value);
            if (Number.isNaN(num)) {
              return;
            }
            onChange(
              clamp(
                num,
                typeof min === 'number' ? min : -Number.MAX_VALUE,
                typeof max === 'number' ? max : Number.MAX_VALUE,
              ),
            );
            onFinishChange();
          }}
          editable={!disabled}
          style={controlsStyles.text}
        />
      </ControlInput>
    </ControlBackground>
  );
};
