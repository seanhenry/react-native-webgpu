/**
 * @format
 */

import 'fast-text-encoding';
import {AppRegistry, Platform} from 'react-native';
import App from './src/App';
import {name as appName} from './app.json';
import {install, Backends, defaultBackends} from 'react-native-webgpu';
import {
  ENABLE_THREADS,
  install as installExperimental,
} from 'react-native-webgpu-experimental';
import 'react-native-webgpu-three';
import {isEmulatorSync} from 'react-native-device-info';

defaultBackends.current =
  Platform.OS === 'android' && isEmulatorSync() ? Backends.GL : Backends.All;
install();
installExperimental();
if (ENABLE_THREADS) {
  global.reactNativeWebGPUThreads.spawn({
    bundleId: 'myThread',
    threadId: 'myThread-0',
  });
}
AppRegistry.registerComponent(appName, () => App);
