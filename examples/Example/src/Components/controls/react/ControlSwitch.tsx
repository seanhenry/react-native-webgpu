import {Switch} from 'react-native';
import {useEffect, useState} from 'react';
import {Observable} from '../../../utils/observable';
import {ControlBackground} from './ControlBackground';
import {ControlLabel} from './ControlLabel';
import {ControlInput} from './ControlInput';
import {controlForegroundColor} from './controlsStyles.ts';

type ControlSwitchProps = {
  onChange: (value: boolean) => void;
  initialValue?: boolean;
  title: string;
  disabled: boolean;
  observable: Observable<boolean>;
};

export const ControlSwitch = ({
  onChange,
  initialValue,
  title,
  disabled,
  observable,
}: ControlSwitchProps) => {
  const [value, setValue] = useState(!!initialValue);
  useEffect(() => observable.add(setValue), [observable]);

  return (
    <ControlBackground>
      <ControlLabel text={title} />
      <ControlInput>
        <Switch
          disabled={disabled}
          value={value}
          onValueChange={val => {
            setValue(val);
            onChange(val);
          }}
          trackColor={{true: controlForegroundColor}}
        />
      </ControlInput>
    </ControlBackground>
  );
};
