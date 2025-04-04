import * as THREE from 'three';

import {Square} from '../../Components/Square';
import {globalStyles} from '../../Components/globalStyles';
import {ThreeWebGpuView, ThreeWebGpuViewProps} from 'react-native-webgpu-three';

import {OrbitControls} from 'three/addons/controls/OrbitControls.js';
import WebGPURenderer from 'three/addons/renderers/webgpu/WebGPURenderer.js';
import {RGBELoader} from 'three/addons/loaders/RGBELoader.js';
import PMREMGenerator from 'three/addons/renderers/common/extras/PMREMGenerator.js';
import {GLTFLoader as ThreeGTLFLoader} from 'three/addons/loaders/GLTFLoader.js';
import {useState} from 'react';
import {ActivityIndicator, StyleSheet, View} from 'react-native';
import {THREE_EXAMPLES_BASE_URL} from '../../utils/constants';

export const GLTFLoader = () => {
  const [isLoading, setLoading] = useState(true);
  const onCreateSurface: ThreeWebGpuViewProps['onCreateSurface'] = async ({
    context,
    rendererParameters,
  }) => {
    let camera: THREE.Camera, scene: THREE.Scene, renderer: WebGPURenderer;

    init();

    function init() {
      camera = new THREE.PerspectiveCamera(
        45,
        context.width / context.height,
        0.25,
        20,
      );
      camera.position.set(-1.8, 0.6, 2.7);

      scene = new THREE.Scene();

      new RGBELoader()
        .setPath(`${THREE_EXAMPLES_BASE_URL}/textures/equirectangular/`)
        .load('royal_esplanade_1k.hdr', function (texture) {
          const pmremGenerator = new PMREMGenerator(renderer);

          // @ts-expect-error PMREMGenerator is not correctly typed
          const envMap = pmremGenerator.fromEquirectangular(texture).texture;

          // scene.background = envMap;
          scene.environment = envMap;

          texture.dispose();
          // @ts-expect-error PMREMGenerator is not correctly typed
          pmremGenerator.dispose();

          // This method does not reload correctly:
          // texture.mapping = THREE.EquirectangularReflectionMapping;
          // // texture.minFilter = THREE.LinearMipmapLinearFilter;
          // // texture.generateMipmaps = true;
          //
          // scene.background = texture;
          // scene.environment = texture;

          render();

          // model

          const loader = new ThreeGTLFLoader().setPath(
            `${THREE_EXAMPLES_BASE_URL}/models/gltf/DamagedHelmet/glTF/`,
          );
          loader.load('DamagedHelmet.gltf', function (gltf) {
            setLoading(false);
            scene.add(gltf.scene);

            render();
          });
        });

      renderer = new WebGPURenderer({antialias: true, ...rendererParameters});
      // renderer.setPixelRatio( window.devicePixelRatio );
      // renderer.setSize( width, height );
      renderer.toneMapping = THREE.ACESFilmicToneMapping;
      // container.appendChild( renderer.domElement );

      const controls = new OrbitControls(camera, renderer.domElement);
      controls.addEventListener('change', render); // use if there is no animation loop
      controls.minDistance = 2;
      controls.maxDistance = 10;
      controls.target.set(0, 0, -0.2);
      controls.update();
    }

    async function render() {
      await renderer.renderAsync(scene, camera);
      context.presentSurface();
    }
    return () => {
      renderer.dispose();
    };
  };
  return (
    <Square>
      <ThreeWebGpuView
        onCreateSurface={onCreateSurface}
        style={globalStyles.fill}
      />
      {isLoading && (
        <View style={[StyleSheet.absoluteFill, globalStyles.centerContents]}>
          <ActivityIndicator />
        </View>
      )}
    </Square>
  );
};
