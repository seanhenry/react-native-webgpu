import {PropsWithChildren} from 'react';
import {View} from 'react-native';
import {controlsStyles} from './controlsStyles';

type ControlBackgroundProps = PropsWithChildren<{dark?: boolean}>;

export const ControlBackground = ({children, dark}: ControlBackgroundProps) => {
  return (
    <View
      style={[
        controlsStyles.controlsBackgroundColor,
        controlsStyles.background,
        dark ? controlsStyles.controlsBackgroundColorDark : null,
      ]}>
      {children}
    </View>
  );
};
