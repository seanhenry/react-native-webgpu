const transformer = require('react-native-webgpu/wgsl-babel-transformer');

module.exports.transform = function (options) {
  if (options.filename.endsWith('.gltf')) {
    options.src = `const contents = ${options.src};
export default contents;`;
  }
  return transformer.transform(options);
};
