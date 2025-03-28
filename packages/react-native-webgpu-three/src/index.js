import 'three/examples/jsm/nodes/math/MathNode';
import './nodeBuilderPatch';
import './constNodePatch';
import './rendererPatch';
import 'fast-text-encoding';
import {makeLoggingProxy} from './debugLogging';
import {Image} from './Image';

export {ThreeWebGpuView} from './ThreeWebGpuView';
export {enableDebugLogging} from './debugLogging';

global.document = makeLoggingProxy(
  {
    createElementNS: makeLoggingProxy(
      (ns, name) => {
        if (name === 'img') {
          return new Image();
        }
      },
      ['document', 'createElementNS'],
    ),
    addEventListener: makeLoggingProxy(() => {}, [
      'document',
      'addEventListener',
    ]),
  },
  ['document'],
);

// GLTFLoader.js:2589
global.navigator = makeLoggingProxy(
  {
    userAgent: 'ReactNative',
    scheduling: undefined,
  },
  ['navigator'],
);

// CacheNode.js:26
global.parent = makeLoggingProxy({}, ['parent']);
