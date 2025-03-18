import {exec, mkdir, replaceFileContents, rm, spawn} from '../utils/shell.js';
import path from 'node:path';
import {
  EXAMPLES_DIR,
  WEBGPU_DIR,
  WEBGPU_EXPERIMENTAL_DIR,
  WEBGPU_EXPERIMENTAL_VERSION,
  WEBGPU_THREE_DIR,
  WEBGPU_THREE_VERSION,
  WEBGPU_VERSION,
} from '../utils/config.js';
import fs from 'node:fs/promises';

export async function generateProject({
  reactNativeVersion,
  directory,
  fd,
  print,
}) {
  const g = new ProjectGenerator({reactNativeVersion, directory, fd, print});
  await g.prepareProject();
  await g.generateProject();
  await g.installRnVersionSpecificPackages();
  await g.installWebGPUPackages();
  await g.copyExamplesSource();
  await g.reverseMonorepoConfig();
  await g.setMinAndroidVersion();
  await g.allowUnsecureSockets();
  await g.checkJavaScriptErrors();
}

class ProjectGenerator {
  constructor({reactNativeVersion, directory, fd, print}) {
    this.reactNativeVersion = reactNativeVersion;
    this.directory = directory;
    this.fd = fd;
    this.print = print;
  }

  async prepareProject() {
    await rm('rf', this.directory);
    await mkdir('p', this.directory);
  }

  async generateProject() {
    this.print('Generating examples project');
    await spawn(
      `npx`,
      [
        '-y',
        '@react-native-community/cli@latest',
        'init',
        'Example',
        '--version',
        this.reactNativeVersion,
        '--skip-install',
        '--directory',
        this.jsDirectory,
        '--skip-git-init',
      ],
      {stdio: this.stdio},
    );
    await this.exec('touch yarn.lock');
    await this.exec('yarn set version 3.6.4');
  }

  async installRnVersionSpecificPackages() {
    this.print('Installing specific packages for react native');
    await this._reactNativeVersionSpecificSetup();
  }

  async installWebGPUPackages() {
    this.print('Installing webgpu packages');
    await this
      .exec(`yarn add "${WEBGPU_EXPERIMENTAL_DIR}/react-native-webgpu-experimental-${WEBGPU_EXPERIMENTAL_VERSION}.tgz" \
      "${WEBGPU_DIR}/react-native-webgpu-${WEBGPU_VERSION}.tgz" \
      "${WEBGPU_THREE_DIR}/react-native-webgpu-three-${WEBGPU_THREE_VERSION}.tgz"`);
  }

  async copyExamplesSource() {
    this.print('Copying examples config and source');
    await this.exec(`cp -r "${EXAMPLES_DIR}/babel.config.js" \
      "${EXAMPLES_DIR}/index.js" \
      "${EXAMPLES_DIR}/metro.config.js" \
      "${EXAMPLES_DIR}/tsconfig.json" \
      "${EXAMPLES_DIR}/.eslintignore" \
      "${EXAMPLES_DIR}/.eslintrc.js" \
      "${EXAMPLES_DIR}/.prettierignore" \
      "${EXAMPLES_DIR}/.prettierrc.js" \
      "${EXAMPLES_DIR}/babel" \
      "${EXAMPLES_DIR}/src" \
      .`);
  }

  async reverseMonorepoConfig() {
    this.print('Changing node_modules relative path in tsconfig.json');
    await replaceFileContents(
      path.join(this.jsDirectory, 'tsconfig.json'),
      /..\/..\/node_modules/g,
      'node_modules',
    );
  }

  async setMinAndroidVersion() {
    this.print('Setting android minSdkVersion to 27');
    await replaceFileContents(
      path.join(this.jsDirectory, 'android/build.gradle'),
      /minSdkVersion = [0-9]+/,
      'minSdkVersion = 27',
    );
  }

  async allowUnsecureSockets() {
    this.print(
      'Setting android network config to allow unsecure socket callback',
    );
    const NETWORK_CONFIG_FILE = path.join(
      this.jsDirectory,
      'android/app/src/main/res/xml/network_security_config.xml',
    );
    const NETWORK_XML_CONTENTS = `<?xml version="1.0" encoding="utf-8"?>
<network-security-config>
  <domain-config cleartextTrafficPermitted="true">
    <domain includeSubdomains="false">127.0.0.1</domain>
  </domain-config>
</network-security-config>
`;
    await fs.mkdir(path.dirname(NETWORK_CONFIG_FILE), {recursive: true});
    await fs.writeFile(NETWORK_CONFIG_FILE, NETWORK_XML_CONTENTS);
    await replaceFileContents(
      path.join(this.jsDirectory, 'android/app/src/main/AndroidManifest.xml'),
      /<application/,
      '<application android:networkSecurityConfig="@xml/network_security_config"',
    );
  }

  async checkJavaScriptErrors() {
    this.print('Checking for TS errors');
    await this.spawn('yarn', ['tsc', '--noEmit']);

    this.print('Linting');
    await this.spawn('yarn', ['eslint', '.']);
  }

  async exec(command) {
    return exec(command, this.shellOptions);
  }

  async spawn(command, args) {
    return spawn(command, args ?? [], this.shellOptions);
  }

  /**
   * Each react native version may have different requirements due to breaking changes, especially with libraries like react navigation and reanimated.
   */
  async _reactNativeVersionSpecificSetup() {
    switch (this.reactNativeVersion) {
      case '0.78.0': {
        // Note, >0.81.0 introduces "_interopRequireDefault is not a function" error when `unstable_enablePackageExports` is `true` in metro.config.js (required for three.js)
        await this.exec(
          'npm pkg set resolutions.metro=0.81.0; \
        npm pkg set resolutions.metro-config=0.81.0; \
        npm pkg set resolutions.metro-core=0.81.0; \
        npm pkg set resolutions.metro-resolver=0.81.0; \
        npm pkg set resolutions.metro-runtime=0.81.0; \
        npm pkg set resolutions.metro-source-map=0.81.0;',
        );

        await this.exec(
          'yarn add "@gltf-transform/core@4.0.10" \
        "@react-navigation/native@7.0.14" \
        "@react-navigation/native-stack@7.2.0" \
        "lodash@4.17.21" \
        "react-native-gesture-handler@2.24.0" \
        "react-native-launch-arguments@4.1.0" \
        "react-native-reanimated@3.17.0" \
        "react-native-safe-area-context@5.2.0" \
        "react-native-screens@4.9.0" \
        "teapot@1.0.0" \
        "three@0.166.1" \
        "wgpu-matrix@3.0.2" \
        "@babel/plugin-transform-export-namespace-from@7.24.7" \
        "@types/lodash@4.17.7" \
        "@types/three@0.166.0" \
        "fast-text-encoding@1.0.6"',
        );
        break;
      }
      case '0.77.1': {
        // Note, >0.81.0 introduces "_interopRequireDefault is not a function" error when `unstable_enablePackageExports` is `true` in metro.config.js (required for three.js)
        await this.exec(
          'npm pkg set resolutions.metro=0.81.0; \
        npm pkg set resolutions.metro-config=0.81.0; \
        npm pkg set resolutions.metro-core=0.81.0; \
        npm pkg set resolutions.metro-resolver=0.81.0; \
        npm pkg set resolutions.metro-runtime=0.81.0; \
        npm pkg set resolutions.metro-source-map=0.81.0;',
        );

        await this.exec(
          'yarn add "@gltf-transform/core@4.0.10" \
        "@react-navigation/native@7.0.14" \
        "@react-navigation/native-stack@7.2.0" \
        "lodash@4.17.21" \
        "react-native-gesture-handler@2.23.1" \
        "react-native-launch-arguments@4.0.4" \
        "react-native-reanimated@3.16.7" \
        "react-native-safe-area-context@5.2.0" \
        "react-native-screens@4.7.0" \
        "teapot@1.0.0" \
        "three@0.166.1" \
        "wgpu-matrix@3.0.1" \
        "@babel/plugin-transform-export-namespace-from@7.24.7" \
        "@types/lodash@4.17.7" \
        "@types/three@0.166.0" \
        "fast-text-encoding@1.0.6"',
        );
        break;
      }
      case '0.76.7': {
        await this.exec(
          'yarn add "@gltf-transform/core@4.0.10" \
          "@react-navigation/native@7.0.3" \
          "@react-navigation/native-stack@7.1.0" \
          "lodash@4.17.21" \
          "react-native-gesture-handler@2.21.2" \
          "react-native-launch-arguments@4.0.2" \
          "react-native-reanimated@3.16.2" \
          "react-native-safe-area-context@4.14.0" \
          "react-native-screens@4.2.0" \
          "teapot@1.0.0" \
          "three@0.166.1" \
          "wgpu-matrix@3.0.2" \
          "@babel/plugin-transform-export-namespace-from@7.24.7" \
          "@types/lodash@4.17.7" \
          "@types/three@0.166.0" \
          "fast-text-encoding@1.0.6"',
        );
        break;
      }
      default:
        throw new Error(
          `Unknown react native version ${this.reactNativeVersion}`,
        );
    }
  }

  get shellOptions() {
    return {cwd: this.jsDirectory, stdio: this.stdio};
  }
  get jsDirectory() {
    return path.join(this.directory, 'Example');
  }
  get stdio() {
    return ['inherit', this.fd, this.fd];
  }
}
