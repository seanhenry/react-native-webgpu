module.exports = {
  presets: ['module:@react-native/babel-preset'],
  plugins: [
    ...require('react-native-webgpu-three/babel').plugins,
    ['babel-plugin-inline-import', {extensions: ['.wgsl']}],
  ],
};
