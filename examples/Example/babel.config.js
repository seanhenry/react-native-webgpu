module.exports = {
  presets: ['module:@react-native/babel-preset'],
  plugins: [
    'react-native-webgpu/babel-plugin-transform-wgsl',
    ...require('react-native-webgpu-three/babel').plugins,
    'react-native-reanimated/plugin',
  ],
};
