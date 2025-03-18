import {exec, rm} from '../utils/shell.js';
import {
  WEBGPU_DIR,
  WEBGPU_EXPERIMENTAL_DIR,
  WEBGPU_THREE_DIR,
  WEBGPU_VERSION,
} from '../utils/config.js';
import path from 'node:path';

export async function pack() {
  await clearCache();
  await packLibrary('react-native-webgpu', WEBGPU_VERSION, WEBGPU_DIR);
  await Promise.all([
    packLibrary('react-native-webgpu-three', '0.0.1', WEBGPU_THREE_DIR),
    packLibrary(
      'react-native-webgpu-experimental',
      '0.0.1',
      WEBGPU_EXPERIMENTAL_DIR,
    ),
  ]);
}

async function clearCache() {
  console.log('Clearing webgpu packages from yarn caches');
  await Promise.all([
    exec(`rm -f "$(yarn config get cacheFolder)"/react-native-webgpu*`),
    exec(`rm -f "$(yarn config get globalFolder)"/cache/react-native-webgpu*`),
  ]);
}

async function packLibrary(name, version, directory) {
  console.log(`Packing ${name}`);
  await rm('f', path.join(directory, `${name}-${version}.tgz`));
  await exec(`npm pack`, {cwd: directory});
}
