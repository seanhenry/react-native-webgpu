import {List, ListProps} from './List';
import {
  type NavigationProp,
  RouteProp,
  useNavigation,
  useRoute,
} from '@react-navigation/native';
import type {Routes} from '../types/navigationTypes';
import type {Example, Examples} from './ExampleScreen';
import {useSafeAreaInsets} from 'react-native-safe-area-context';

export function Examples() {
  const navigation = useNavigation<NavigationProp<Routes, 'example'>>();
  const {params} = useRoute<RouteProp<Routes, 'examples'>>();
  const insets = useSafeAreaInsets();

  return (
    <List<Example>
      insets={{...insets, top: 0}}
      onPressItem={item => navigation.navigate('example', {name: item.title})}
      sections={sections[params.name]}
    />
  );
}

const sections: Record<Examples, ListProps<Example>['sections']> = {
  WebGPUSamples: [
    {
      title: 'Basic Graphics',
      data: [
        {title: 'HelloTriangle'},
        {title: 'HelloTriangleMSAA'},
        {title: 'RotatingCube'},
        {title: 'TwoCubes'},
        {title: 'TexturedCube'},
        {title: 'InstancedCube'},
        {title: 'FractalCube'},
        {title: 'CubeMap'},
      ],
    },
    {
      title: 'GPGPU',
      data: [
        {title: 'ComputeBoids'},
        {title: 'GameOfLife'},
        {title: 'BitonicSort'},
      ],
    },
    {
      title: 'WebGPU Features',
      data: [
        {title: 'SamplerParameters'},
        {title: 'ReversedZ'},
        {title: 'RenderBundles'},
        {title: 'OcclusionQueries'},
      ],
    },
    {
      title: 'Graphics Techniques',
      data: [
        {title: 'Cameras'},
        {title: 'NormalMap'},
        {title: 'ShadowMapping'},
        {title: 'DeferredRendering'},
        {title: 'ParticlesHDR'},
        {title: 'ImageBlur'},
        {title: 'Cornell'},
        {title: 'ABuffer'},
        {title: 'SkinnedMesh'},
        {title: 'VolumeRendering'},
        {title: 'Wireframe'},
      ],
    },
  ],
  Three: [
    {
      title: 'Three.js',
      data: [{title: 'HelloThree'}, {title: 'Clearcoat'}, {title: 'Clipping'}],
    },
    {
      title: 'Camera',
      data: [{title: 'LogarithmicDepthBuffer'}],
    },

    {
      title: 'Compute',
      data: [{title: 'ComputeGeometry'}, {title: 'ComputeParticles'}],
    },
    {
      title: 'Loader',
      data: [{title: 'GLTFLoader'}],
    },
  ],
  InHouse: [
    {
      title: 'InHouse',
      data: [
        {title: 'FlakesTexture'},
        {title: 'Crop'},
        {title: 'Video'},
        {title: 'Outlines'},
        {title: 'Portal'},
        {title: 'Thread'},
        {title: 'AdapterInfo'},
        {title: 'ExceedDeviceLimits'},
        {title: 'CWTriangle'},
        {title: 'ClearBuffer'},
        {title: 'ResizeCanvas'},
        {title: 'AnimateCanvas'},
      ],
    },
  ],
};
