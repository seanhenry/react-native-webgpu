import {ThreeWebGpuView, ThreeWebGpuViewProps} from 'react-native-webgpu-three';
import {globalStyles} from '../../Components/globalStyles';
import {useStats} from '../../Components/stats/useStats';

import * as THREE from 'three';
import {Scene, Vector3} from 'three';

import {FontLoader} from 'three/addons/loaders/FontLoader.js';
import {TextGeometry} from 'three/addons/geometries/TextGeometry.js';
import WebGPURenderer from 'three/addons/renderers/webgpu/WebGPURenderer.js';
import {THREE_EXAMPLES_BASE_URL} from '../threeConstants';
import {ActivityIndicator, StyleSheet, View} from 'react-native';
import {HudContainer} from '../../Components/stats/HudContainer';
import {useEffect, useState} from 'react';

export const LogarithmicDepthBuffer = () => {
  const {stats, Stats} = useStats();
  const [scene, setScene] = useState<Scene | null>(null);
  useEffect(() => {
    initScene().then(setScene).catch(console.error);
  }, []);
  const onCreateSurface = async (
    {
      context,
      rendererParameters,
      requestAnimationFrame,
    }: Parameters<ThreeWebGpuViewProps['onCreateSurface']>[0],
    type: 'normal' | 'logzbuf',
  ) => {
    // 1 micrometer to 100 billion light years in one scene, with 1 unit = 1 meter?  preposterous!  and yet...
    const NEAR = 1e-6,
      FAR = 1e27;
    let SCREEN_WIDTH = context.width;
    let SCREEN_HEIGHT = context.height;
    const screensplit = 0.25,
      screensplit_right = 0;
    const mouse = [0.5, 0.5];
    let zoompos = -100;
    const minzoomspeed = 0.015;
    let zoomspeed = minzoomspeed;

    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    let object: any = {};

    // Generate a number of text labels, from 1µm in size up to 100,000,000 light years
    // Try to use some descriptive real-world examples of objects at each scale

    init().then(animate);

    async function init() {
      // Initialize two copies of the same scene, one with normal z-buffer and one with logarithmic z-buffer
      object = await initView(scene!, type === 'logzbuf');
    }

    // eslint-disable-next-line @typescript-eslint/no-shadow
    async function initView(scene: Scene, logDepthBuf: boolean) {
      const camera = new THREE.PerspectiveCamera(
        50,
        SCREEN_WIDTH / SCREEN_HEIGHT,
        NEAR,
        FAR,
      );
      scene.add(camera);

      const renderer = new WebGPURenderer({
        antialias: true,
        logarithmicDepthBuffer: logDepthBuf,
        ...rendererParameters,
      });

      await renderer.init();

      return {
        renderer: renderer,
        scene: scene,
        camera: camera,
      };
    }

    function updateRendererSizes() {
      // Recalculate size for both renderers when screen size or split location changes

      SCREEN_WIDTH = context.width;
      SCREEN_HEIGHT = context.height;

      object.renderer.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
      object.camera.aspect = SCREEN_WIDTH / SCREEN_HEIGHT;
      object.camera.updateProjectionMatrix();
      object.camera.setViewOffset(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SCREEN_WIDTH,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
      );
    }

    function animate() {
      if (type === 'logzbuf') {
        stats.begin();
      }
      requestAnimationFrame(animate);

      // Put some limits on zooming
      const minzoom = labeldata[0].size * labeldata[0].scale;
      const maxzoom =
        labeldata[labeldata.length - 1].size *
        labeldata[labeldata.length - 1].scale *
        100;
      let damping = Math.abs(zoomspeed) > minzoomspeed ? 0.95 : 1.0;

      // Zoom out faster the further out you go
      const zoom = THREE.MathUtils.clamp(
        Math.pow(Math.E, zoompos),
        minzoom,
        maxzoom,
      );
      zoompos = Math.log(zoom);

      // Slow down quickly at the zoom limits
      if (
        (zoom === minzoom && zoomspeed < 0) ||
        (zoom === maxzoom && zoomspeed > 0)
      ) {
        damping = 0.85;
      }

      zoompos += zoomspeed;
      zoomspeed *= damping;

      object.camera.position.x =
        Math.sin(0.5 * Math.PI * (mouse[0] - 0.5)) * zoom;
      object.camera.position.y =
        Math.sin(0.25 * Math.PI * (mouse[1] - 0.5)) * zoom;
      object.camera.position.z =
        Math.cos(0.5 * Math.PI * (mouse[0] - 0.5)) * zoom;
      object.camera.lookAt(new Vector3(-0.75 * zoom, 0, 0));

      // Update renderer sizes if the split has changed
      if (screensplit_right !== 1 - screensplit) {
        updateRendererSizes();
      }

      object.renderer.render(object.scene, object.camera);

      context.presentSurface();
      if (type === 'logzbuf') {
        stats.end();
      }
    }
  };
  const onCreateSurfaceNormal: ThreeWebGpuViewProps['onCreateSurface'] =
    props => onCreateSurface(props, 'normal');
  const onCreateSurfaceLogZBuffer: ThreeWebGpuViewProps['onCreateSurface'] =
    props => onCreateSurface(props, 'logzbuf');

  return scene ? (
    <View style={globalStyles.fill}>
      <ThreeWebGpuView
        onCreateSurface={onCreateSurfaceNormal}
        style={styles.normal}
      />
      <ThreeWebGpuView
        onCreateSurface={onCreateSurfaceLogZBuffer}
        style={styles.logZBuffer}
      />
      <HudContainer>
        <Stats />
      </HudContainer>
    </View>
  ) : (
    <View style={[globalStyles.fill, globalStyles.centerContents]}>
      <ActivityIndicator />
    </View>
  );
};

const styles = StyleSheet.create({
  normal: {
    flex: 1,
  },
  logZBuffer: {
    flex: 2,
  },
});

const labeldata = [
  {size: 0.01, scale: 0.0001, label: 'microscopic (1µm)'}, // FIXME - triangulating text fails at this size, so we scale instead
  {size: 0.01, scale: 0.1, label: 'minuscule (1mm)'},
  {size: 0.01, scale: 1.0, label: 'tiny (1cm)'},
  {size: 1, scale: 1.0, label: 'child-sized (1m)'},
  {size: 10, scale: 1.0, label: 'tree-sized (10m)'},
  {size: 100, scale: 1.0, label: 'building-sized (100m)'},
  {size: 1000, scale: 1.0, label: 'medium (1km)'},
  {size: 10000, scale: 1.0, label: 'city-sized (10km)'},
  {size: 3400000, scale: 1.0, label: 'moon-sized (3,400 Km)'},
  {size: 12000000, scale: 1.0, label: 'planet-sized (12,000 km)'},
  {size: 1400000000, scale: 1.0, label: 'sun-sized (1,400,000 km)'},
  {size: 7.47e12, scale: 1.0, label: 'solar system-sized (50Au)'},
  {size: 9.4605284e15, scale: 1.0, label: 'gargantuan (1 light year)'},
  {size: 3.08567758e16, scale: 1.0, label: 'ludicrous (1 parsec)'},
  {size: 1e19, scale: 1.0, label: 'mind boggling (1000 light years)'},
];

async function initScene() {
  const loader = new FontLoader();
  const font = await loader.loadAsync(
    `${THREE_EXAMPLES_BASE_URL}/fonts/helvetiker_regular.typeface.json`,
  );

  const scene = new THREE.Scene();

  scene.add(new THREE.AmbientLight(0x777777));

  const light = new THREE.DirectionalLight(0xffffff, 3);
  light.position.set(100, 100, 100);
  scene.add(light);

  const materialargs = {
    color: 0xffffff,
    specular: 0x050505,
    shininess: 50,
    emissive: 0x000000,
  };

  const geometry = new THREE.SphereGeometry(0.5, 24, 12);

  for (let i = 0; i < labeldata.length; i++) {
    const scale = labeldata[i].scale || 1;

    const labelgeo = new TextGeometry(labeldata[i].label, {
      font,
      size: labeldata[i].size,
      depth: labeldata[i].size / 2,
    });

    labelgeo.computeBoundingSphere();

    // center text
    labelgeo.translate(-labelgeo.boundingSphere!.radius, 0, 0);

    // @ts-expect-error Types are wrong
    materialargs.color = new THREE.Color().setHSL(Math.random(), 0.5, 0.5);

    const material = new THREE.MeshPhongMaterial(materialargs);

    const group = new THREE.Group();
    group.position.z = -labeldata[i].size * scale;
    scene.add(group);

    const textmesh = new THREE.Mesh(labelgeo, material);
    textmesh.scale.set(scale, scale, scale);
    textmesh.position.z = -labeldata[i].size * scale;
    textmesh.position.y = (labeldata[i].size / 4) * scale;
    group.add(textmesh);

    const dotmesh = new THREE.Mesh(geometry, material);
    dotmesh.position.y = (-labeldata[i].size / 4) * scale;
    dotmesh.scale.multiplyScalar(labeldata[i].size * scale);
    group.add(dotmesh);
  }

  return scene;
}
