module.exports = function transformWgsl() {
  return {
    name: 'babel-plugin-transform-wgsl',
    parserOverride(code, options, parse) {
      const { sourceFileName } = options;
      if (sourceFileName?.endsWith('.wgsl')) {
        code = `const contents = \`${code}\`
export default contents;`;
      }
      return parse(code, options);
    },
  };
};
