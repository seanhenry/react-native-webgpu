/**
 * @format
 */

import 'fast-text-encoding';
import {AppRegistry} from 'react-native';
import App from './src/App';
import {name as appName} from './app.json';
import {install, ENABLE_THREADS} from 'react-native-webgpu';
import 'react-native-webgpu-three';

install();
if (ENABLE_THREADS) {
  global.reactNativeWebGPUThreads.spawn({
    bundleId: 'myThread',
    threadId: 'myThread-0',
  });
}
AppRegistry.registerComponent(appName, () => App);
