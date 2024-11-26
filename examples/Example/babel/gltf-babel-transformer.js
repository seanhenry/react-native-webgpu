const babelTransformer = require('react-native-webgpu/wgsl-babel-transformer');

module.exports = {...babelTransformer};
module.exports.transform = function (options) {
  if (options.filename.endsWith('.gltf')) {
    options.src = `const contents = ${options.src};
export default contents;`;
  }
  return babelTransformer.transform(options);
};
