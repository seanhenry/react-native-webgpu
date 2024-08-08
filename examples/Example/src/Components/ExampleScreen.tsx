import React, { type ComponentType } from 'react';
import { HelloTriangle } from '../WebGPUSamples/BasicGraphics/HelloTriangle/HelloTriangle';
import { CubeMap } from '../WebGPUSamples/BasicGraphics/CubeMap/CubeMap';
import { HelloTriangleMSAA } from '../WebGPUSamples/BasicGraphics/HelloTriangleMSAA/HelloTriangleMSAA';
import { RotatingCube } from '../WebGPUSamples/BasicGraphics/RotatingCube/RotatingCube';
import { TwoCubes } from '../WebGPUSamples/BasicGraphics/TwoCubes/TwoCubes';
import { TexturedCube } from '../WebGPUSamples/BasicGraphics/TexturedCube/TexturedCube';
import { InstancedCube } from '../WebGPUSamples/BasicGraphics/InstancedCube/InstancedCube';
import { FractalCube } from '../WebGPUSamples/BasicGraphics/FractalCube/FractalCube';
import { BitonicSort } from '../WebGPUSamples/GPGPU/BitonicSort/BitonicSort';
import { ComputeBoids } from '../WebGPUSamples/GPGPU/ComputeBoids/ComputeBoids';
import { GameOfLife } from '../WebGPUSamples/GPGPU/GameOfLife/GameOfLife';
import { type RouteProp, useRoute } from '@react-navigation/native';
import type { Routes } from '../types/navigationTypes';
import { Cameras } from '../WebGPUSamples/GraphicsTechniques/Cameras/Cameras';
import { HelloThree } from '../Three/HelloThree/HelloThree';
import { GLTFLoader } from '../Three/GLTFLoader/GLTFLoader';

export type Example =
  // BasicGraphics
  | 'HelloTriangle'
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
  // GraphicsTechniques
  | 'Cameras'
  // Three
  | 'HelloThree'
  | 'GLTFLoader'

const examples: Record<Example, ComponentType> = {
  // BasicGraphics
  HelloTriangle: HelloTriangle,
  HelloTriangleMSAA: HelloTriangleMSAA,
  RotatingCube: RotatingCube,
  TwoCubes: TwoCubes,
  TexturedCube: TexturedCube,
  InstancedCube: InstancedCube,
  FractalCube: FractalCube,
  CubeMap: CubeMap,
  // GPGPU
  ComputeBoids: ComputeBoids,
  GameOfLife: GameOfLife,
  BitonicSort: BitonicSort,
  // GraphicsTechniques
  Cameras: Cameras,
  // Three.js
  HelloThree: HelloThree,
  GLTFLoader: GLTFLoader,
}

export function ExampleScreen() {
  const {params} = useRoute<RouteProp<Routes, 'example'>>();
  const Example = examples[params.name];
  return <Example />
}
