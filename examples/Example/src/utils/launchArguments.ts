import {LaunchArguments} from 'react-native-launch-arguments';

interface LaunchArguments {
  example?: string;
}

export const launchArguments = LaunchArguments.value<LaunchArguments>();
