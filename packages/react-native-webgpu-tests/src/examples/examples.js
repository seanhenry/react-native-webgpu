import {PromptBuilder} from '../utils/prompt-builder.js';
import {exec, mkdir} from '../utils/shell.js';
import chalk from 'chalk';
import {SocketListener} from './socket-listener.js';
import {PRODUCTS_DIR, TEST_DIR} from '../utils/config.js';
import {generateExamplesHtml} from './generate-examples-html.js';
import {EXAMPLES_TO_TEST} from './examples-to-test.js';
import {Queue} from '../utils/queue.js';
import fs from 'node:fs/promises';
import path from 'node:path';

const configurations = ['Release'];
const config = await new PromptBuilder()
  .platforms()
  .reactNativeVersions()
  .architectures()
  .prompt();

const IOS_BUNDLE_ID = 'org.reactjs.native.example.Example';
const ANDROID_BUNDLE_ID = 'com.example';
const OUT_DIR = `${TEST_DIR}/screenshots/${new Date().toISOString()}`;

await mkdir('p', OUT_DIR);

function print(message) {
  console.log(message);
}

function printSuccess(message) {
  console.log(chalk.green(message));
}

function printFailure(message) {
  console.log(chalk.red(message));
}

const listener = new SocketListener();
await listener.startListening();

const iosQueue = new Queue();
const androidQueue = new Queue();
let callbackId = 0;

for (const rnVersion of config.reactNativeVersions) {
  await mkdir(
    'p',
    `${OUT_DIR}/${rnVersion}/ios/newarch`,
    `${OUT_DIR}/${rnVersion}/ios/oldarch`,
    `${OUT_DIR}/${rnVersion}/android/newarch`,
    `${OUT_DIR}/${rnVersion}/android/oldarch`,
  );
  for (const architecture of config.architectures) {
    for (const configuration of configurations) {
      const IOS_APP = `${PRODUCTS_DIR}/Example-${configuration}-${architecture}-${rnVersion}.app`;
      const ANDROID_APP = `${PRODUCTS_DIR}/Example-${configuration}-${architecture}-${rnVersion}.apk`;

      if (config.platforms.includes('ios')) {
        const id = ++callbackId;
        iosQueue.push(async () => {
          await exec(`xcrun simctl terminate booted "${IOS_BUNDLE_ID}"`).catch(
            () => {},
          );
          await exec(`xcrun simctl install booted "${IOS_APP}"`);
        });

        for (const example of EXAMPLES_TO_TEST) {
          iosQueue.push(async () => {
            await exec(
              `xcrun simctl launch booted "${IOS_BUNDLE_ID}" -example "${example}" -callbackid "${id}"`,
            );
            try {
              await listener.waitForExample(example, id);
              printSuccess(
                `[Screenshot] ios-${rnVersion}-${architecture} ${example}`,
              );
            } catch (error) {
              printFailure(
                `[Failed] ios-${rnVersion}-${architecture} ${example}`,
              );
            }
            await exec(
              `xcrun simctl io booted screenshot "${OUT_DIR}/${rnVersion}/ios/${architecture}/${example}.png"`,
            );
            await exec(`xcrun simctl terminate booted "${IOS_BUNDLE_ID}"`);
          });
        }
      }
      if (config.platforms.includes('android')) {
        const id = ++callbackId;
        androidQueue.push(async () => {
          await exec('adb reverse tcp:8888 tcp:8888');
          await exec(`adb install -r "${ANDROID_APP}"`);
        });

        for (const example of EXAMPLES_TO_TEST) {
          androidQueue.push(async () => {
            await exec(
              `adb shell am start -n "${ANDROID_BUNDLE_ID}/.MainActivity" --es "example" "${example}" --es "callbackid" "${id}"`,
            );
            try {
              await listener.waitForExample(example, id);
              printSuccess(
                `[Screenshot] android-${rnVersion}-${architecture} ${example}`,
              );
            } catch (error) {
              printFailure(
                `[Failed] android-${rnVersion}-${architecture} ${example}`,
              );
            }
            await exec(
              `adb exec-out screencap -p > "${OUT_DIR}/${rnVersion}/android/${architecture}/${example}.png"`,
            );

            await exec(`adb shell am force-stop "${ANDROID_BUNDLE_ID}"`);
          });
        }
      }
    }
  }
}

await Promise.all([iosQueue.execute(), androidQueue.execute()]);

listener.close();

print('Generating html');
const html = await generateExamplesHtml({
  outDir: OUT_DIR,
  rnVersions: config.reactNativeVersions,
  architectures: config.architectures,
  platforms: config.platforms,
});

print(`Writing results to ${OUT_DIR}`);
await fs.writeFile(path.join(OUT_DIR, 'index.html'), html);
await exec(`open "${OUT_DIR}/index.html"`);
