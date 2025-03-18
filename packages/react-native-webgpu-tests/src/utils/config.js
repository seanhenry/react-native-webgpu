import fs from 'node:fs';
import path from 'node:path';

const DIR_NAME = path.dirname(import.meta.url.replace('file://', ''));
const ROOT_DIR = path.join(DIR_NAME, '..', '..', '..', '..');
const PACKAGES_DIR = path.join(ROOT_DIR, 'packages');
export const EXAMPLES_DIR = path.join(ROOT_DIR, 'examples', 'Example');
export const WEBGPU_DIR = path.join(PACKAGES_DIR, 'react-native-webgpu');
export const WEBGPU_THREE_DIR = path.join(
  PACKAGES_DIR,
  'react-native-webgpu-three',
);
export const WEBGPU_EXPERIMENTAL_DIR = path.join(
  PACKAGES_DIR,
  'react-native-webgpu-experimental',
);

function parsePackageJson(directory) {
  return JSON.parse(
    fs.readFileSync(path.join(directory, 'package.json'), {encoding: 'utf8'}),
  );
}

const RN_WEBGPU_PACKAGE_JSON = parsePackageJson(WEBGPU_DIR);
const RN_WEBGPU_THREE_PACKAGE_JSON = parsePackageJson(WEBGPU_THREE_DIR);
const RN_WEBGPU_EXPERIMENTAL_PACKAGE_JSON = parsePackageJson(
  WEBGPU_EXPERIMENTAL_DIR,
);

export const REACT_NATIVE_VERSIONS = Object.freeze(['0.78.0', '0.77.1']);
export const WEBGPU_VERSION = RN_WEBGPU_PACKAGE_JSON.version;
export const WEBGPU_THREE_VERSION = RN_WEBGPU_THREE_PACKAGE_JSON.version;
export const WEBGPU_EXPERIMENTAL_VERSION =
  RN_WEBGPU_EXPERIMENTAL_PACKAGE_JSON.version;
export const IOS_DESTINATION = 'OS=18.0,name=iPhone 16';

export const PWD = process.cwd();
export const TEST_DIR = `${PWD}/.test`;
export const LOGS_DIR = path.join(TEST_DIR, 'logs');
export const PRODUCTS_DIR = `${TEST_DIR}/products`;

export function isNewArchitecture(architecture) {
  return architecture === 'newarch';
}
