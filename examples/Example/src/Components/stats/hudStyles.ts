import {StyleSheet} from 'react-native';

export const hudStyles = StyleSheet.create({
  container: {
    ...StyleSheet.absoluteFillObject,
    bottom: undefined,
    backgroundColor: '#00000080',
    paddingVertical: 2,
    paddingHorizontal: 4,
    gap: 2,
  },
  text: {
    padding: 0,
    margin: 0,
    color: '#FFF',
  },
});
