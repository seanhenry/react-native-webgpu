import {LaunchArguments as RNLaunchArguments} from 'react-native-launch-arguments';

interface LaunchArguments {
  example?: string;
  callbackid?: string;
}

export const launchArguments = RNLaunchArguments.value<LaunchArguments>();
