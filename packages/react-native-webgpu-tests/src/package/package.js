import {PromptBuilder} from '../utils/prompt-builder.js';
import {pack} from './pack.js';
import {LOGS_DIR, PRODUCTS_DIR, TEST_DIR} from '../utils/config.js';
import path from 'node:path';
import {generateProject} from './generate-project.js';
import {buildIos, cleanIos} from './build-ios.js';
import {buildAndroid, cleanAndroid} from './build-android.js';
import {Queue} from '../utils/queue.js';
import {withLog} from '../utils/log.js';
import {mkdir, mv, rm} from '../utils/shell.js';

const config = await new PromptBuilder()
  .skipPack()
  .platforms()
  .reactNativeVersions()
  .architectures()
  .configurations()
  .prompt();
const projectGenerations = [];
const buildQueue = new Queue();

await rm('rf', LOGS_DIR);
await mkdir('p', LOGS_DIR);

const productsDir = PRODUCTS_DIR;
await mv(
  productsDir,
  `${process.env.HOME}/.Trash/products-${new Date().toISOString()}`,
).catch(() => {});
await mkdir('p', productsDir);

if (!config.skipPack) {
  await pack();
}

for (const reactNativeVersion of config.reactNativeVersions) {
  const directory = path.join(TEST_DIR, reactNativeVersion);
  const jsDirectory = path.join(directory, 'Example');
  const iOSDirectory = path.join(jsDirectory, 'ios');
  const androidDirectory = path.join(jsDirectory, 'android');

  projectGenerations.push(async () => {
    await withLog(
      `${reactNativeVersion}-generateProject.txt`,
      `[RN ${reactNativeVersion}]`,
      async ({print, fd}) => {
        await generateProject({reactNativeVersion, directory, fd, print});
      },
    );
  });

  for (const configuration of config.configurations) {
    for (const architecture of config.architectures) {
      if (config.platforms.includes('ios')) {
        buildQueue.push(async () => {
          await withLog(
            `${reactNativeVersion}-${architecture}-${configuration}-ios-build.txt`,
            `[RN ${reactNativeVersion} iOS ${configuration} ${architecture}]`,
            async ({print, stdio}) => {
              await cleanIos({directory: iOSDirectory, print});
              await buildIos({
                architecture,
                configuration,
                directory: iOSDirectory,
                productsDir,
                reactNativeVersion,
                stdio,
                print,
              });
            },
          );
        });
      }
      if (config.platforms.includes('android')) {
        buildQueue.push(async () => {
          await withLog(
            `${reactNativeVersion}-${architecture}-${configuration}-android-build.txt`,
            `[RN ${reactNativeVersion} Android ${configuration} ${architecture}]`,
            async ({print, stdio}) => {
              await cleanAndroid({directory: androidDirectory, print});
              await buildAndroid({
                architecture,
                configuration,
                directory: androidDirectory,
                productsDir,
                reactNativeVersion,
                stdio,
                print,
              });
            },
          );
        });
      }
    }
  }
}

await Promise.all(projectGenerations.map(task => task()));
// Can't currently run builds in parallel because of metro
await buildQueue.execute();
