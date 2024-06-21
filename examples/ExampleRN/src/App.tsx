import * as React from 'react';

import {StyleSheet, View} from 'react-native';
import {HelloTriangle} from "./WebGPUSamples/src/HelloTriangle/HelloTriangle";
import { RotatingCube } from './WebGPUSamples/src/RotatingCube/RotatingCube';

export default function App() {
  return (
    <View style={styles.container}>
      <HelloTriangle />
      <RotatingCube />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
});
