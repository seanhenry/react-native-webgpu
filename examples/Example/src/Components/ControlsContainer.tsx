import {PropsWithChildren} from 'react';
import {StyleSheet, View} from 'react-native';

export const ControlsContainer = ({children}: PropsWithChildren) => {
  return <View style={styles.controls}>{children}</View>;
};

const styles = StyleSheet.create({
  controls: {
    position: 'absolute',
    top: 8,
    left: 8,
    right: 8,
  },
});
