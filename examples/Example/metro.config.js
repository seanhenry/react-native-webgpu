const {getDefaultConfig, mergeConfig} = require('@react-native/metro-config');
const path = require('path');
const webGpuConfig = require('react-native-webgpu/metro');
const threeConfig = require('react-native-webgpu-three/metro');

const root = path.resolve(__dirname, '../..');
const defaultConfig = getDefaultConfig(__dirname);

/**
 * Metro configuration
 * https://reactnative.dev/docs/metro
 *
 * @type {import('@react-native/metro-config').MetroConfig}
 */
const config = {
  watchFolders: [root],
  resolver: {
    resolveRequest: threeConfig.resolver.resolveRequest,
    unstable_enablePackageExports: true,
    sourceExts: [
      ...defaultConfig.resolver.sourceExts,
      ...webGpuConfig.resolver.sourceExts,
      'gltf',
    ],
  },
  transformer: {
    babelTransformerPath: require.resolve('./babel/gltf-babel-transformer'),
  },
};

module.exports = mergeConfig(defaultConfig, config);
