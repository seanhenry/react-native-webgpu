import {ABuffer} from '../WebGPUSamples/GraphicsTechniques/ABuffer/ABuffer';
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
import {ImageBlur} from '../WebGPUSamples/GraphicsTechniques/ImageBlur/ImageBlur';
import {Cornell} from '../WebGPUSamples/GraphicsTechniques/Cornell/Cornell';
import {LogarithmicDepthBuffer} from '../Three/LogarithmicDepthBuffer/LogarithmicDepthBuffer';
import {Clearcoat} from '../Three/Clearcoat/Clearcoat';
import {Clipping} from '../Three/Clipping/Clipping';
import {ComputeGeometry} from '../Three/ComputeGeometry/ComputeGeometry';
import {ComputeParticles} from '../Three/ComputeParticles/ComputeParticles';
import {FlakesTexture} from '../InHouse/FlakesTexture';
import {VolumeRendering} from '../WebGPUSamples/GraphicsTechniques/VolumeRendering/VolumeRendering';
import {SkinnedMesh} from '../WebGPUSamples/GraphicsTechniques/SkinnedMesh/SkinnedMesh';
import {Video} from '../InHouse/Video';
import {Thread} from '../InHouse/Thread';
import {ParticlesHDR} from '../WebGPUSamples/GraphicsTechniques/ParticlesHDR/ParticlesHDR';
import {Wireframe} from '../WebGPUSamples/GraphicsTechniques/Wireframe/Wireframe';
import {AdapterInfo} from '../InHouse/AdapterInfo';
import {ExceedDeviceLimits} from '../InHouse/ExceedDeviceLimits';
import {Crop} from '../InHouse/Crop';
import {Outlines} from '../InHouse/Outlines/Outlines';
import {Portal} from '../InHouse/Portal/Portal';
import {CWTriangle} from '../InHouse/CWTriangle/CWTriangle';

export type Examples = 'WebGPUSamples' | 'Three' | 'InHouse';

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
  | 'ParticlesHDR'
  | 'ImageBlur'
  | 'Cornell'
  | 'ABuffer'
  | 'SkinnedMesh'
  | 'VolumeRendering'
  | 'Wireframe'
  // Three
  | 'HelloThree'
  | 'Clearcoat'
  | 'Clipping'
  // Camera
  | 'LogarithmicDepthBuffer'
  // Compute
  | 'ComputeGeometry'
  | 'ComputeParticles'
  // Loader
  | 'GLTFLoader'
  // InHouse
  | 'FlakesTexture'
  | 'Crop'
  | 'Video'
  | 'Outlines'
  | 'Portal'
  | 'Thread'
  | 'AdapterInfo'
  | 'ExceedDeviceLimits'
  | 'CWTriangle';

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
  ParticlesHDR,
  ImageBlur,
  Cornell,
  ABuffer,
  SkinnedMesh,
  VolumeRendering,
  Wireframe,
  // Three.js
  HelloThree,
  Clearcoat,
  Clipping,
  // Camera
  LogarithmicDepthBuffer,
  // Compute
  ComputeGeometry,
  ComputeParticles,
  // Loader
  GLTFLoader,
  // InHouse
  FlakesTexture,
  Crop,
  Video,
  Outlines,
  Portal,
  Thread,
  AdapterInfo,
  ExceedDeviceLimits,
  CWTriangle,
};

export function ExampleScreen() {
  const {params} = useRoute<RouteProp<Routes, 'example'>>();
  const Example = examples[params.name];
  return <Example />;
}
