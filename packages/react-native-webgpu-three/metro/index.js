const path = require('path');
const threeRoot = path.resolve(require.resolve('three'), '../..');
const rnThreeRoot = path.resolve(
  require.resolve('react-native-webgpu-three'),
  '../..',
);

module.exports = {
  resolver: {
    resolveRequest: (context, moduleName, platform) => {
      const resolved = context.resolveRequest(context, moduleName, platform);
      if (
        resolved.filePath?.endsWith('three/examples/jsm/capabilities/WebGPU.js')
      ) {
        resolved.filePath = path.join(
          rnThreeRoot,
          resolved.filePath.replace(/^.*\/three\//, 'src/'),
        );
      } else if (moduleName.startsWith('three/addons/')) {
        resolved.filePath = moduleName.replace(
          'three/addons/',
          path.resolve(threeRoot, 'examples/jsm') + '/',
        );
      }
      return resolved;
    },
  },
};
