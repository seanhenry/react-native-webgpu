import type { ComponentType } from 'react';
import * as React from 'react';

import { StyleSheet, View } from 'react-native';
import { HelloTriangle } from './WebGPUSamples/src/HelloTriangle/HelloTriangle';
import { HelloTriangleMSAA } from './WebGPUSamples/src/HelloTriangleMSAA/HelloTriangleMSAA';
import { RotatingCube } from './WebGPUSamples/src/RotatingCube/RotatingCube';
import { TwoCubes } from './WebGPUSamples/src/TwoCubes/TwoCubes';
import { TexturedCube } from './WebGPUSamples/src/TexturedCube/TexturedCube';
import { InstancedCube } from './WebGPUSamples/src/InstancedCube/InstancedCube';

type Example = 'HelloTriangle'
  | 'HelloTriangleMSAA'
  | 'RotatingCube'
  | 'TwoCubes'
  | 'TexturedCube'
  | 'InstancedCube'

// const example: string = 'HelloTriangle'
// const example: string = 'HelloTriangleMSAA'
// const example: string = 'RotatingCube'
// const example: Example = 'TwoCubes'
// const example: Example = 'TexturedCube'
const example: Example = 'InstancedCube'

const examples: Record<Example, ComponentType> = {
  HelloTriangle: HelloTriangle,
  HelloTriangleMSAA: HelloTriangleMSAA,
  RotatingCube: RotatingCube,
  TwoCubes: TwoCubes,
  TexturedCube: TexturedCube,
  InstancedCube: InstancedCube,
}

export default function App() {
  const Component = examples[example];
  return (
    <View style={styles.container}>
      <Component />
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