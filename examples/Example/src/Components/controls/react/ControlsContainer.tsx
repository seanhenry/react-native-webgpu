import {PropsWithChildren} from 'react';
import {Keyboard, ScrollView, StyleSheet} from 'react-native';
import {useSafeAreaInsets} from 'react-native-safe-area-context';
import {controlGroupBackgroundColor} from './controlsStyles';

export const ControlsContainer = ({children}: PropsWithChildren) => {
  const insets = useSafeAreaInsets();
  return (
    <ScrollView
      style={styles.scrollView}
      alwaysBounceVertical={false}
      onTouchStart={Keyboard.dismiss}
      contentInset={{bottom: insets.bottom}}>
      {children}
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  scrollView: {
    flex: 1,
    backgroundColor: controlGroupBackgroundColor,
  },
});
