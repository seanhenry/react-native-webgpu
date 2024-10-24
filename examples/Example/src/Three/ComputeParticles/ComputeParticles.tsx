import {Square} from '../../Components/Square';
import {
  PointerEvent,
  ThreeWebGpuView,
  ThreeWebGpuViewProps,
} from 'react-native-webgpu-three';
import {globalStyles} from '../../Components/globalStyles';

import * as THREE from 'three';
import {
  tslFn,
  uniform,
  texture,
  instanceIndex,
  float,
  vec3,
  storage,
  If,
  SpriteNodeMaterial,
  ComputeNode,
} from 'three/examples/jsm/nodes/Nodes.js';

import {OrbitControls} from 'three/addons/controls/OrbitControls.js';
import {Camera, Scene} from 'three';
import WebGPURenderer from 'three/addons/renderers/webgpu/WebGPURenderer.js';
import StorageInstancedBufferAttribute from 'three/examples/jsm/renderers/common/StorageInstancedBufferAttribute.js';
import {useControls} from '../../Components/controls/react/useControls';
import {useStats} from '../../Components/stats/useStats';
import {HudContainer} from '../../Components/stats/HudContainer';
import {HudText} from '../../Components/stats/HudText';
import {useRef} from 'react';
import {TextInput} from 'react-native';
import {THREE_EXAMPLES_BASE_URL} from '../../utils/constants';
import {useHudText} from '../../Components/stats/useHudText';

export const ComputeParticles = () => {
  const {gui, Controls} = useControls();
  const {stats, Stats} = useStats();
  const {setText, HudText} = useHudText();
  const textRef = useRef<TextInput | null>(null);
  const onCreateSurface: ThreeWebGpuViewProps['onCreateSurface'] = ({
    context,
    rendererParameters,
    eventsAdapter,
  }) => {
    const particleCount = 1000000;

    const gravity = uniform(-0.0098);
    const bounce = uniform(0.8);
    const friction = uniform(0.99);
    const size = uniform(0.12);

    const clickPosition = uniform(new THREE.Vector3());

    let camera: Camera, scene: Scene, renderer: WebGPURenderer;
    let controls: OrbitControls;
    let computeParticles: ComputeNode[];

    init();

    function init() {
      const {width, height} = context;

      camera = new THREE.PerspectiveCamera(50, width / height, 0.1, 1000);
      camera.position.set(15, 30, 15);

      scene = new THREE.Scene();

      // textures

      const textureLoader = new THREE.TextureLoader();
      const map = textureLoader.load(
        `${THREE_EXAMPLES_BASE_URL}/textures/sprite1.png`,
      );

      //

      const createBuffer = () =>
        storage(
          new StorageInstancedBufferAttribute(particleCount, 3),
          'vec3',
          particleCount,
        );

      const positionBuffer = createBuffer();
      const velocityBuffer = createBuffer();
      const colorBuffer = createBuffer();

      // compute

      // @ts-expect-error Ignoring type errors
      const computeInit = tslFn(() => {
        const position = positionBuffer.element(instanceIndex);
        const color = colorBuffer.element(instanceIndex);

        const randX = instanceIndex.hash();
        const randY = instanceIndex.add(2).hash();
        const randZ = instanceIndex.add(3).hash();

        position.x = randX.mul(100).add(-50);
        // @ts-expect-error Ignoring type errors
        position.y = 0; // randY.mul( 10 );
        position.z = randZ.mul(100).add(-50);

        color.assign(vec3(randX, randY, randZ));
        // @ts-expect-error Ignoring type errors
      })().compute(particleCount);

      //

      // @ts-expect-error Ignoring type errors
      const computeUpdate = tslFn(() => {
        const position = positionBuffer.element(instanceIndex);
        const velocity = velocityBuffer.element(instanceIndex);

        velocity.addAssign(vec3(0.0, gravity, 0.0));
        position.addAssign(velocity);

        velocity.mulAssign(friction);

        // floor

        If(position.y.lessThan(0), () => {
          // @ts-expect-error Ignoring type errors
          position.y = 0;
          velocity.y = velocity.y.negate().mul(bounce);

          // floor friction

          velocity.x = velocity.x.mul(0.9);
          velocity.z = velocity.z.mul(0.9);
        });
      });

      // @ts-expect-error Ignoring type errors
      computeParticles = computeUpdate().compute(particleCount);

      // create nodes

      const textureNode = texture(map);

      // create particles

      const particleMaterial = new SpriteNodeMaterial();
      particleMaterial.colorNode = textureNode.mul(
        colorBuffer.element(instanceIndex),
      );
      // @ts-expect-error Ignoring type errors
      particleMaterial.positionNode = positionBuffer.toAttribute();
      particleMaterial.scaleNode = size;
      particleMaterial.depthWrite = false;
      particleMaterial.depthTest = true;
      particleMaterial.transparent = true;

      const particles = new THREE.Mesh(
        new THREE.PlaneGeometry(1, 1),
        particleMaterial,
      );
      // @ts-expect-error Ignoring type errors
      particles.count = particleCount;
      particles.frustumCulled = false;
      scene.add(particles);

      //

      const helper = new THREE.GridHelper(60, 40, 0x303030, 0x303030);
      scene.add(helper);

      const geometry = new THREE.PlaneGeometry(1000, 1000);
      geometry.rotateX(-Math.PI / 2);

      const plane = new THREE.Mesh(
        geometry,
        new THREE.MeshBasicMaterial({visible: false}),
      );
      scene.add(plane);

      const raycaster = new THREE.Raycaster();
      const pointer = new THREE.Vector2();

      //

      renderer = new WebGPURenderer({
        antialias: true,
        trackTimestamp: true,
        ...rendererParameters,
      });
      renderer.setAnimationLoop(animate);

      //

      renderer.compute(computeInit);

      // click event

      // @ts-expect-error Ignoring type errors
      const computeHit = tslFn(() => {
        const position = positionBuffer.element(instanceIndex);
        const velocity = velocityBuffer.element(instanceIndex);

        const dist = position.distance(clickPosition);
        const direction = position.sub(clickPosition).normalize();
        const distArea = float(6).sub(dist).max(0);

        const power = distArea.mul(0.01);
        const relativePower = power.mul(instanceIndex.hash().mul(0.5).add(0.5));

        velocity.assign(velocity.add(direction.mul(relativePower)));
        // @ts-expect-error Ignoring type errors
      })().compute(particleCount);

      //

      function onMove(event: PointerEvent) {
        pointer.set(
          (event.clientX / context.pointWidth) * 2 - 1,
          -(event.clientY / context.pointHeight) * 2 + 1,
        );

        raycaster.setFromCamera(pointer, camera);

        const intersects = raycaster.intersectObjects([plane], false);

        if (intersects.length > 0) {
          const {point} = intersects[0];

          // move to uniform

          clickPosition.value.copy(point);
          clickPosition.value.y = -1;

          // compute

          renderer.compute(computeHit);
        }
      }

      // events

      eventsAdapter.addEventListener('pointermove', onMove);

      //

      controls = new OrbitControls(camera, renderer.domElement);
      controls.minDistance = 5;
      controls.maxDistance = 200;
      controls.target.set(0, 0, 0);
      controls.update();
      controls.enabled = false;

      // gui

      gui.add(gravity, 'value', -0.0098, 0, 0.0001).name('gravity');
      gui.add(bounce, 'value', 0.1, 1, 0.01).name('bounce');
      gui.add(friction, 'value', 0.96, 0.99, 0.01).name('friction');
      gui.add(size, 'value', 0.12, 0.5, 0.01).name('size');
      gui.add(controls, 'enabled').name('Enable controls');
      gui.draw();
    }

    async function animate() {
      stats.begin();

      await renderer.computeAsync(computeParticles);

      await renderer.renderAsync(scene, camera);

      // throttle the logging

      if (renderer.hasFeature('timestamp-query')) {
        if (renderer.info.render.calls % 5 === 0) {
          setText(`Compute ${
            renderer.info.compute.frameCalls
          } pass in ${renderer.info.compute.timestamp.toFixed(6)}ms
Draw ${
            renderer.info.render.drawCalls
          } pass in ${renderer.info.render.timestamp.toFixed(6)}ms`);
        }
      } else {
        setText('Timestamp queries not supported');
      }
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
        <HudText />
      </HudContainer>
      <Controls />
    </>
  );
};
