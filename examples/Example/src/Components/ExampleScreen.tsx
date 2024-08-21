import {type ComponentType} from 'react';
import {HelloTriangle} from '../WebGPUSamples/BasicGraphics/HelloTriangle/HelloTriangle';
import {CubeMap} from '../WebGPUSamples/BasicGraphics/CubeMap/CubeMap';
import {HelloTriangleMSAA} from '../WebGPUSamples/BasicGraphics/HelloTriangleMSAA/HelloTriangleMSAA';
import {RotatingCube} from '../WebGPUSamples/BasicGraphics/RotatingCube/RotatingCube';
import {TwoCubes} from '../WebGPUSamples/BasicGraphics/TwoCubes/TwoCubes';
import {TexturedCube} from '../WebGPUSamples/BasicGraphics/TexturedCube/TexturedCube';
import {InstancedCube} from '../WebGPUSamples/BasicGraphics/InstancedCube/InstancedCube';
import {FractalCube} from '../WebGPUSamples/BasicGraphics/FractalCube/FractalCube';
import {BitonicSort} from '../WebGPUSamples/GPGPU/BitonicSort/BitonicSort';
import {ComputeBoids} from '../WebGPUSamples/GPGPU/ComputeBoids/ComputeBoids';
import {GameOfLife} from '../WebGPUSamples/GPGPU/GameOfLife/GameOfLife';
import {type RouteProp, useRoute} from '@react-navigation/native';
import type {Routes} from '../types/navigationTypes';
import {Cameras} from '../WebGPUSamples/GraphicsTechniques/Cameras/Cameras';
import {HelloThree} from '../Three/HelloThree/HelloThree';
import {GLTFLoader} from '../Three/GLTFLoader/GLTFLoader';
import {SamplerParameters} from '../WebGPUSamples/WebGPUFeatures/SamplerParameters/SamplerParameters';
import {ReversedZ} from '../WebGPUSamples/WebGPUFeatures/ReversedZ/ReversedZ';
import {RenderBundles} from '../WebGPUSamples/WebGPUFeatures/RenderBundles/RenderBundles';
import {OcclusionQueries} from '../WebGPUSamples/WebGPUFeatures/OcclusionQueries/OcclusionQueries';
import {NormalMap} from '../WebGPUSamples/GraphicsTechniques/NormalMap/NormalMap';
import {ShadowMapping} from '../WebGPUSamples/GraphicsTechniques/ShadowMapping/ShadowMapping';
import {DeferredRendering} from '../WebGPUSamples/GraphicsTechniques/DeferredRendering/DeferredRendering';
import {ImageBlur} from '../WebGPUSamples/GraphicsTechniques/ImageBlur/ImageBlur.tsx';

export type Examples = 'WebGPUSamples' | 'Three';

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
  // WebGPUFeatures
  | 'SamplerParameters'
  | 'ReversedZ'
  | 'RenderBundles'
  | 'OcclusionQueries'
  // GraphicsTechniques
  | 'Cameras'
  | 'NormalMap'
  | 'ShadowMapping'
  | 'DeferredRendering'
  | 'ImageBlur'
  // Three
  | 'HelloThree'
  | 'GLTFLoader';

const examples: Record<Example, ComponentType> = {
  // BasicGraphics
  HelloTriangle,
  HelloTriangleMSAA,
  RotatingCube,
  TwoCubes,
  TexturedCube,
  InstancedCube,
  FractalCube,
  CubeMap,
  // GPGPU
  ComputeBoids,
  GameOfLife,
  BitonicSort,
  // WebGPUFeatures
  SamplerParameters,
  ReversedZ,
  RenderBundles,
  OcclusionQueries,
  // GraphicsTechniques
  Cameras,
  NormalMap,
  ShadowMapping,
  DeferredRendering,
  ImageBlur,
  // Three.js
  HelloThree,
  GLTFLoader,
};

export function ExampleScreen() {
  const {params} = useRoute<RouteProp<Routes, 'example'>>();
  const Example = examples[params.name];
  return <Example />;
}
