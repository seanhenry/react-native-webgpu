import {Switch, Text} from 'react-native';
import {useState} from 'react';

type ToggleProps = {
  onChange: (value: boolean) => void;
  initialValue?: boolean;
};

export const Toggle = ({onChange, initialValue}: ToggleProps) => {
  const [value, setValue] = useState(!!initialValue);
  return (
    <>
      <Text>Use render bundles</Text>
      <Switch
        value={value}
        onValueChange={val => {
          setValue(val);
          onChange(val);
        }}
      />
    </>
  );
};
