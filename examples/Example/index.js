/**
 * @format
 */

import 'react-native-webgpu';
import 'react-native-webgpu-three';
import 'fast-text-encoding';
import {AppRegistry} from 'react-native';
import App from './App';
import {name as appName} from './app.json';

AppRegistry.registerComponent(appName, () => App);
