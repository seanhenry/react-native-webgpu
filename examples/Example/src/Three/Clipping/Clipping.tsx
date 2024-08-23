import {ThreeWebGpuView, ThreeWebGpuViewProps} from 'react-native-webgpu-three';
import {globalStyles} from '../../Components/globalStyles';
import {Square} from '../../Components/Square';

import * as THREE from 'three';
import {Camera, Scene} from 'three';

import {OrbitControls} from 'three/addons/controls/OrbitControls.js';
import WebGPURenderer from 'three/addons/renderers/webgpu/WebGPURenderer.js';
import {MeshPhongNodeMaterial} from 'three/examples/jsm/nodes/materials/Materials.js';
import {useStats} from '../../Components/stats/useStats';
import {useControls} from '../../Components/controls/react/useControls';
import {HudContainer} from '../../Components/stats/HudContainer';

export const Clipping = () => {
  const {stats, Stats} = useStats();
  const {gui, Controls} = useControls();
  const onCreateSurface: ThreeWebGpuViewProps['onCreateSurface'] = async ({
    context,
    rendererParameters,
  }) => {
    let camera: Camera,
      scene: Scene,
      renderer: WebGPURenderer,
      startTime: number,
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      object: any;

    init();

    function init() {
      camera = new THREE.PerspectiveCamera(
        36,
        context.width / context.height,
        0.25,
        16,
      );

      camera.position.set(0, 1.3, 3);

      scene = new THREE.Scene();

      // Lights

      scene.add(new THREE.AmbientLight(0xcccccc));

      const spotLight = new THREE.SpotLight(0xffffff, 60);
      spotLight.angle = Math.PI / 5;
      spotLight.penumbra = 0.2;
      spotLight.position.set(2, 3, 3);
      spotLight.castShadow = true;
      spotLight.shadow.camera.near = 3;
      spotLight.shadow.camera.far = 10;
      spotLight.shadow.mapSize.width = 2048;
      spotLight.shadow.mapSize.height = 2048;
      spotLight.shadow.bias = -0.002;
      spotLight.shadow.radius = 4;
      scene.add(spotLight);

      const dirLight = new THREE.DirectionalLight(0x55505a, 3);
      dirLight.position.set(0, 3, 0);
      dirLight.castShadow = true;
      dirLight.shadow.camera.near = 1;
      dirLight.shadow.camera.far = 10;

      dirLight.shadow.camera.right = 1;
      dirLight.shadow.camera.left = -1;
      dirLight.shadow.camera.top = 1;
      dirLight.shadow.camera.bottom = -1;

      dirLight.shadow.mapSize.width = 1024;
      dirLight.shadow.mapSize.height = 1024;
      scene.add(dirLight);

      // ***** Clipping planes: *****

      const localPlane = new THREE.Plane(new THREE.Vector3(0, -1, 0), 0.8);
      const localPlane2 = new THREE.Plane(new THREE.Vector3(0, 0, -1), 0.1);
      const globalPlane = new THREE.Plane(new THREE.Vector3(-1, 0, 0), 0.1);

      // Geometry

      const material = new MeshPhongNodeMaterial({
        color: 0x80ee10,
        shininess: 0,
        side: THREE.DoubleSide,

        // ***** Clipping setup (material): *****
        clippingPlanes: [localPlane, localPlane2],
        clipShadows: true,
        alphaToCoverage: true,
        clipIntersection: true,
      });

      const geometry = new THREE.TorusKnotGeometry(0.4, 0.08, 95, 20);

      object = new THREE.Mesh(geometry, material);
      object.castShadow = true;
      scene.add(object);

      const ground = new THREE.Mesh(
        new THREE.PlaneGeometry(9, 9, 1, 1),
        new MeshPhongNodeMaterial({color: 0xa0adaf, shininess: 150}),
      );

      ground.rotation.x = -Math.PI / 2; // rotates X/Y to X/Z
      ground.receiveShadow = true;
      scene.add(ground);

      // Renderer

      renderer = new WebGPURenderer({antialias: true, ...rendererParameters});
      renderer.shadowMap.enabled = true;
      renderer.setAnimationLoop(animate);

      // ***** Clipping setup (renderer): *****
      const globalPlanes = [globalPlane];
      const Empty = Object.freeze([]);

      renderer.clippingPlanes = Empty; // GUI sets it to globalPlanes
      renderer.localClippingEnabled = true;

      // Controls
      const controls = new OrbitControls(camera, renderer.domElement);
      controls.target.set(0, 1, 0);
      controls.update();

      // GUI

      const props = {
          alphaToCoverage: true,
        },
        folderLocal = gui.addFolder('Local Clipping'),
        propsLocal = {
          get Enabled() {
            return renderer.localClippingEnabled;
          },
          set Enabled(v) {
            renderer.localClippingEnabled = v;
          },

          get Shadows() {
            return material.clipShadows;
          },
          set Shadows(v) {
            material.clipShadows = v;
          },

          get Intersection() {
            return material.clipIntersection;
          },

          set Intersection(v) {
            material.clipIntersection = v;
          },

          get Plane() {
            return localPlane.constant;
          },
          set Plane(v) {
            localPlane.constant = v;
          },
        },
        folderGlobal = gui.addFolder('Global Clipping'),
        propsGlobal = {
          get Enabled() {
            return renderer.clippingPlanes !== Empty;
          },
          set Enabled(v) {
            renderer.clippingPlanes = v ? globalPlanes : Empty;
          },

          get Plane() {
            return globalPlane.constant;
          },
          set Plane(v) {
            globalPlane.constant = v;
          },
        };

      gui.add(props, 'alphaToCoverage').onChange(function (value) {
        ground.material.alphaToCoverage = value;
        ground.material.needsUpdate = true;

        material.alphaToCoverage = value;
        material.needsUpdate = true;
      });

      folderLocal.add(propsLocal, 'Enabled');
      folderLocal.add(propsLocal, 'Shadows');
      folderLocal.add(propsLocal, 'Intersection');
      folderLocal.add(propsLocal, 'Plane', 0.3, 1.25);

      folderGlobal.add(propsGlobal, 'Enabled');
      folderGlobal.add(propsGlobal, 'Plane', -0.4, 3);

      gui.draw();

      // Start

      startTime = Date.now();
    }

    function animate(currentTime: number) {
      const time = (currentTime - startTime) / 1000;

      object.position.y = 0.8;
      object.rotation.x = time * 0.5;
      object.rotation.y = time * 0.2;
      object.scale.setScalar(Math.cos(time) * 0.125 + 0.875);

      stats.begin();
      renderer.render(scene, camera);
      context.presentSurface();
      stats.end();
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
      <HudContainer>
        <Stats />
      </HudContainer>
      <Controls />
    </>
  );
};
