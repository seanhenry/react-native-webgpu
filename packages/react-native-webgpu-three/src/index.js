import 'three/examples/jsm/nodes/math/MathNode';
import './nodeBuilderPatch';
import './constNodePatch';
import 'fast-text-encoding'
import { makeLoggingProxy } from './debugLogging';

export { ThreeWebGpuView } from './ThreeWebGpuView'
export { enableDebugLogging } from './debugLogging'

global.document = makeLoggingProxy({
  createElementNS: makeLoggingProxy(() => {}, ['document', 'createElementNS']),
  addEventListener: makeLoggingProxy(() => {}, ['document', 'addEventListener'])
}, ['document']);

// GLTFLoader.js:2589
global.navigator = makeLoggingProxy({
  userAgent: 'ReactNative',
  scheduling: undefined,
}, ['navigator']);

// CacheNode.js:26
global.parent = makeLoggingProxy({}, ['parent']);



