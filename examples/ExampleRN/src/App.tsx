import * as React from 'react';

import {StyleSheet, View} from 'react-native';
import { HelloTriangle } from "./WebGPUSamples/src/HelloTriangle/HelloTriangle";
import { HelloTriangleMSAA } from './WebGPUSamples/src/HelloTriangleMSAA/HelloTriangleMSAA';
import { RotatingCube } from './WebGPUSamples/src/RotatingCube/RotatingCube';

// const example: string = 'HelloTriangle'
// const example: string = 'HelloTriangleMSAA'
const example: string = 'RotatingCube'

export default function App() {
  const renderExample = () => {
    if (example === 'HelloTriangle') {
      return <HelloTriangle />
    } else if (example === 'HelloTriangleMSAA') {
      return <HelloTriangleMSAA />
    } else if (example === 'RotatingCube') {
      return <RotatingCube />
    }
    throw new Error(`Unknown example ${example}`)
  }
  return (
    <View style={styles.container}>
      {renderExample()}
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
