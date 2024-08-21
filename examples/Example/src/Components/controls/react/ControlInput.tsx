import {PropsWithChildren} from 'react';
import {View} from 'react-native';
import {controlsStyles} from './controlsStyles';

type ControlInputProps = PropsWithChildren;

export const ControlInput = ({children}: ControlInputProps) => {
  return <View style={controlsStyles.input}>{children}</View>;
};
