const babelTransformer = require('@react-native/metro-babel-transformer');

module.exports = { ...babelTransformer };
module.exports.transform = function transformWgsl(options) {
  if (options.filename.endsWith('.wgsl')) {
    options.src = `const contents = \`${options.src
      .replaceAll('${', '\\${')
      .replaceAll('`', '\\`')}\`
export default contents;`;
  }
  return babelTransformer.transform(options);
};
