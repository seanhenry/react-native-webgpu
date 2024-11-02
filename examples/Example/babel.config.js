module.exports = {
  presets: ['module:@react-native/babel-preset'],
  plugins: [
    ...require('react-native-webgpu-three/babel').plugins,
    'react-native-reanimated/plugin',
  ],
};
