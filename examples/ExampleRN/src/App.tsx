import type { ComponentType } from 'react';
import * as React from 'react';

import { StyleSheet, View } from 'react-native';
import { HelloTriangle } from './WebGPUSamples/BasicGraphics/HelloTriangle/HelloTriangle';
import { HelloTriangleMSAA } from './WebGPUSamples/BasicGraphics/HelloTriangleMSAA/HelloTriangleMSAA';
import { RotatingCube } from './WebGPUSamples/BasicGraphics/RotatingCube/RotatingCube';
import { TwoCubes } from './WebGPUSamples/BasicGraphics/TwoCubes/TwoCubes';
import { TexturedCube } from './WebGPUSamples/BasicGraphics/TexturedCube/TexturedCube';
import { InstancedCube } from './WebGPUSamples/BasicGraphics/InstancedCube/InstancedCube';
import { FractalCube } from './WebGPUSamples/BasicGraphics/FractalCube/FractalCube';
import { CubeMap } from './WebGPUSamples/BasicGraphics/CubeMap/CubeMap';
import { ComputeBoids } from './WebGPUSamples/GPGPU/ComputeBoids/ComputeBoids';
import { GameOfLife } from './WebGPUSamples/GPGPU/GameOfLife/GameOfLife';
import { BitonicSort } from './WebGPUSamples/GPGPU/BitonicSort/BitonicSort';

type Example = 'HelloTriangle'
  // BasicGraphics
  | 'HelloTriangleMSAA'
  | 'RotatingCube'
  | 'TwoCubes'
  | 'TexturedCube'
  | 'InstancedCube'
  | 'FractalCube'
  | 'CubeMap'
  // GPGPU
  | 'ComputeBoids'
  | 'GameOfLife'
  | 'BitonicSort'

// const example: string = 'HelloTriangle'
// const example: string = 'HelloTriangleMSAA'
// const example: string = 'RotatingCube'
// const example: Example = 'TwoCubes'
// const example: Example = 'TexturedCube'
// const example: Example = 'InstancedCube'
// const example: Example = 'FractalCube'
// const example: Example = 'CubeMap'
// const example: Example = 'ComputeBoids'
// const example: Example = 'GameOfLife'
const example: Example = 'BitonicSort'

const examples: Record<Example, ComponentType> = {
  HelloTriangle: HelloTriangle,
  HelloTriangleMSAA: HelloTriangleMSAA,
  RotatingCube: RotatingCube,
  TwoCubes: TwoCubes,
  TexturedCube: TexturedCube,
  InstancedCube: InstancedCube,
  FractalCube: FractalCube,
  CubeMap: CubeMap,
  ComputeBoids: ComputeBoids,
  GameOfLife: GameOfLife,
  BitonicSort: BitonicSort,
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
