import {Square} from '../../Components/Square';
import {ThreeWebGpuView, ThreeWebGpuViewProps} from 'react-native-webgpu-three';
import {globalStyles} from '../../Components/globalStyles';
import * as THREE from 'three';
import {
  vec3,
  cos,
  sin,
  mat3,
  storage,
  tslFn,
  instanceIndex,
  timerLocal,
} from 'three/examples/jsm/nodes/Nodes.js';

import {GLTFLoader} from 'three/addons/loaders/GLTFLoader.js';

import {OrbitControls} from 'three/addons/controls/OrbitControls.js';
import {Camera, Mesh, Scene} from 'three';
import WebGPURenderer from 'three/addons/renderers/webgpu/WebGPURenderer.js';
import StorageBufferAttribute from 'three/examples/jsm/renderers/common/StorageBufferAttribute.js';
import {ComputeNode} from 'three/examples/jsm/nodes/Nodes.js';
import {THREE_EXAMPLES_BASE_URL} from '../../utils/constants';

export const ComputeGeometry = () => {
  const onCreateSurface: ThreeWebGpuViewProps['onCreateSurface'] = ({
    context,
    rendererParameters,
  }) => {
    let camera: Camera, scene: Scene, renderer: WebGPURenderer;
    let computeUpdate: ComputeNode | ComputeNode[];

    init();

    function init() {
      camera = new THREE.PerspectiveCamera(
        50,
        context.width / context.height,
        0.1,
        10,
      );
      camera.position.set(0, 0, 1);

      scene = new THREE.Scene();
      scene.background = new THREE.Color(0x333333);

      new GLTFLoader().load(
        `${THREE_EXAMPLES_BASE_URL}/models/gltf/LeePerrySmith/LeePerrySmith.glb`,
        function (gltf) {
          const mesh = gltf.scene.children[0] as Mesh;
          mesh.scale.setScalar(0.1);
          mesh.material = new THREE.MeshNormalMaterial();
          scene.add(mesh);

          //

          const positionBaseAttribute = mesh.geometry.attributes.position;
          const normalBaseAttribute = mesh.geometry.attributes.normal;

          // replace geometry attributes for storage buffer attributes

          const positionStorageBufferAttribute = new StorageBufferAttribute(
            // @ts-expect-error Ignoring example type errors
            positionBaseAttribute.count,
            4,
          );
          const normalStorageBufferAttribute = new StorageBufferAttribute(
            // @ts-expect-error Ignoring example type errors
            normalBaseAttribute.count,
            4,
          );

          mesh.geometry.setAttribute(
            'position',
            positionStorageBufferAttribute,
          );
          mesh.geometry.setAttribute('normal', normalStorageBufferAttribute);

          // compute shader

          // @ts-expect-error Ignoring example type errors
          const computeFn = tslFn(() => {
            const positionAttribute = storage(
              // @ts-expect-error Ignoring example type errors
              positionBaseAttribute,
              'vec3',
              positionBaseAttribute.count,
            ).toReadOnly();
            const normalAttribute = storage(
              // @ts-expect-error Ignoring example type errors
              normalBaseAttribute,
              'vec3',
              normalBaseAttribute.count,
            ).toReadOnly();

            const positionStorageAttribute = storage(
              positionStorageBufferAttribute,
              'vec4',
              positionStorageBufferAttribute.count,
            );
            const normalStorageAttribute = storage(
              normalStorageBufferAttribute,
              'vec4',
              normalStorageBufferAttribute.count,
            );

            const time = timerLocal(1);
            const scale = 0.3;

            //

            const position = vec3(positionAttribute.element(instanceIndex));
            const normal = vec3(normalAttribute.element(instanceIndex));

            const theta = sin(time.add(position.y)).mul(scale);

            const c = cos(theta);
            const s = sin(theta);

            const m = mat3(c, 0, s, 0, 1, 0, s.negate(), 0, c);

            const transformed = position.mul(m);
            const transformedNormal = normal.mul(m);

            positionStorageAttribute.element(instanceIndex).assign(transformed);
            normalStorageAttribute
              .element(instanceIndex)
              .assign(transformedNormal);
          });

          // @ts-expect-error Ignoring example type errors
          computeUpdate = computeFn().compute(positionBaseAttribute.count);
        },
      );

      // renderer

      renderer = new WebGPURenderer({antialias: true, ...rendererParameters});
      renderer.setAnimationLoop(animate);

      const controls = new OrbitControls(camera, renderer.domElement);
      controls.minDistance = 0.7;
      controls.maxDistance = 2;
    }

    async function animate() {
      if (computeUpdate) {
        await renderer.computeAsync(computeUpdate);
      }

      renderer.render(scene, camera);
      context.presentSurface();
    }
    return () => {
      renderer?.setAnimationLoop(null);
    };
  };

  return (
    <>
      <Square>
        <ThreeWebGpuView
          onCreateSurface={onCreateSurface}
          style={globalStyles.fill}
        />
      </Square>
    </>
  );
};
