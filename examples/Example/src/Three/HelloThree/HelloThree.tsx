import {CenterSquare} from '../../Components/CenterSquare';
import {globalStyles} from '../../Components/globalStyles';
import {ThreeWebGpuView, ThreeWebGpuViewProps} from 'react-native-webgpu-three';
import * as THREE from 'three';

import WebGPURenderer from 'three/addons/renderers/webgpu/WebGPURenderer.js';

export const HelloThree = () => {
  const onCreateSurface: ThreeWebGpuViewProps['onCreateSurface'] = async ({
    context,
    rendererParameters,
  }) => {
    const {width, height} = context;

    const camera = new THREE.PerspectiveCamera(70, width / height, 0.01, 10);
    camera.position.z = 1;

    const scene = new THREE.Scene();

    const geometry = new THREE.BoxGeometry(0.2, 0.2, 0.2);
    const material = new THREE.MeshNormalMaterial();

    const mesh = new THREE.Mesh(geometry, material);
    scene.add(mesh);

    const renderer = new WebGPURenderer({
      antialias: true,
      ...rendererParameters,
    });
    await renderer.init();

    // animation

    function animate(time: number) {
      mesh.rotation.x = time / 2000;
      mesh.rotation.y = time / 1000;

      renderer.render(scene, camera);
      context.presentSurface();
    }
    renderer.setAnimationLoop(animate);

    return () => {
      renderer.setAnimationLoop(null);
    };
  };
  return (
    <CenterSquare>
      <ThreeWebGpuView
        onCreateSurface={onCreateSurface}
        style={globalStyles.fill}
      />
    </CenterSquare>
  );
};
