import fs from 'node:fs/promises';
import {rm, spawn} from '../utils/shell.js';
import path from 'node:path';
import {isNewArchitecture} from '../utils/config.js';

export async function cleanAndroid({directory, print}) {
  print('Cleaning Android project');
  await rm(
    'rf',
    path.join(directory, 'android/build'),
    path.join(directory, 'android/app/.cxx'),
    path.join(directory, 'android/app/build'),
  );
}

export async function buildAndroid({
  architecture,
  configuration,
  directory,
  stdio,
  productsDir,
  reactNativeVersion,
  print,
}) {
  const appName = `Example-${configuration}-${architecture}-${reactNativeVersion}.apk`;

  print(`Stopping gradle daemon`);
  await spawn('./gradlew', ['--stop'], {cwd: directory, stdio});

  print(`Building Android app`);
  await spawn(
    './gradlew',
    [
      `assemble${configuration}`,
      `-PnewArchEnabled=${isNewArchitecture(architecture) ? '1' : '0'}`,
    ],
    {cwd: directory, stdio},
  );

  print(`Copied ${appName} to ${productsDir}`);
  await fs.rename(
    path.join(
      directory,
      `app/build/outputs/apk/${configuration.toLowerCase()}/app-${configuration.toLowerCase()}.apk`,
    ),
    path.join(productsDir, appName),
  );
}
