import {StyleSheet} from 'react-native';

export const controlGroupBackgroundColor = '#000000';
export const controlBackgroundColor = '#303030';
export const controlForegroundColor = '#2FA1D6';
export const controlsStyles = StyleSheet.create({
  text: {
    flex: 1,
    color: '#FFF',
    paddingVertical: 2,
  },
  background: {
    flexDirection: 'row',
    gap: 4,
    paddingHorizontal: 4,
    paddingVertical: 2,
  },
  input: {
    flex: 2,
    flexDirection: 'row',
    gap: 4,
  },
  controlsBackgroundColor: {
    backgroundColor: '#1a1a1a',
  },
  controlsBackgroundColorDark: {
    backgroundColor: controlGroupBackgroundColor,
  },
});
