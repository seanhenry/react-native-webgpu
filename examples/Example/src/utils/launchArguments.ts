import {LaunchArguments as RNLaunchArguments} from 'react-native-launch-arguments';

interface LaunchArguments {
  example?: string;
}

export const launchArguments = RNLaunchArguments.value<LaunchArguments>();
