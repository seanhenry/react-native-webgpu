import {rm, spawn} from '../utils/shell.js';
import {IOS_DESTINATION, isNewArchitecture} from '../utils/config.js';
import fs from 'node:fs/promises';
import path from 'node:path';

export async function cleanIos({directory, print}) {
  print('Cleaning Xcode project');
  await rm('rf', path.join(directory, 'Pods'), path.join(directory, 'build'));
}

export async function buildIos({
  architecture,
  configuration,
  directory,
  stdio,
  productsDir,
  reactNativeVersion,
  print,
}) {
  const appName = `Example-${configuration}-${architecture}-${reactNativeVersion}.app`;

  print('Installing ruby dependencies');
  await spawn('bundle', {stdio, cwd: directory});

  print('Installing pods');
  await spawn('bundle', ['exec', 'pod', 'install'], {
    env: {
      RCT_NEW_ARCH_ENABLED: isNewArchitecture(architecture) ? '1' : '0',
      ...process.env,
    },
    stdio,
    cwd: directory,
  });

  print('Building iOS app');
  await spawn(
    'xcodebuild',
    [
      '-workspace',
      'Example.xcworkspace',
      '-scheme',
      'Example',
      '-configuration',
      configuration,
      '-destination',
      `'${IOS_DESTINATION}'`,
      '-derivedDataPath',
      'build',
      'build',
    ],
    {
      cwd: directory,
      stdio,
    },
  );

  print(`Copied ${appName} to ${productsDir}`);
  await fs.rename(
    path.join(
      directory,
      `build/Build/Products/${configuration}-iphonesimulator/Example.app`,
    ),
    path.join(productsDir, appName),
  );
}
