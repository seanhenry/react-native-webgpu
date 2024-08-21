import {Keyboard, StyleSheet, View} from 'react-native';
import type {ReactNode} from 'react';

export const Square = ({children}: {children: ReactNode}) => (
  <View style={styles.container} onTouchStart={Keyboard.dismiss}>
    {children}
  </View>
);

const styles = StyleSheet.create({
  container: {
    width: '100%',
    aspectRatio: 1,
  },
});
