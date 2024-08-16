module.exports = {
  presets: ['module:@react-native/babel-preset'],
  plugins: [
    require('react-native-webgpu/babel-plugin-transform-wgsl'),
    ...require('react-native-webgpu-three/babel').plugins,
  ],
};
