import {Observable} from '../../../utils/observable';
import {TextInput} from 'react-native';
import {useEffect, useState} from 'react';
import {ControlBackground} from './ControlBackground';
import {ControlLabel} from './ControlLabel';
import {ControlInput} from './ControlInput';
import {controlsStyles} from './controlsStyles';

type ControlTextInputProps = {
  title: string;
  disabled: boolean;
  initialValue: string;
  observable: Observable<string>;
  onChange: (value: string) => void;
  onFinishChange: () => void;
};

export const ControlTextInput = ({
  title,
  disabled,
  initialValue,
  observable,
  onChange,
  onFinishChange,
}: ControlTextInputProps) => {
  const [value, setValue] = useState(initialValue);
  useEffect(() => observable.add(setValue), [observable]);
  return (
    <ControlBackground>
      <ControlLabel text={title} />
      <ControlInput>
        <TextInput
          onTouchStart={e => e.stopPropagation()} // Prevent keyboard from being dismissed
          keyboardType="default"
          autoCorrect={false}
          autoCapitalize="none"
          value={value}
          onChangeText={onChange}
          onBlur={onFinishChange}
          editable={!disabled}
          style={controlsStyles.text}
        />
      </ControlInput>
    </ControlBackground>
  );
};
