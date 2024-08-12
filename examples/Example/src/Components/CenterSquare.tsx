import {StyleSheet, View} from 'react-native';
import type {ReactNode} from 'react';

export const CenterSquare = ({children}: {children: ReactNode}) => (
  <View style={styles.container}>
    <View style={styles.square}>{children}</View>
  </View>
);

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
  square: {
    width: '100%',
    aspectRatio: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
});
