if (process.argv.includes('package')) {
  await import('./package/package.js');
} else if (process.argv.includes('examples')) {
  await import('./examples/examples.js');
}
