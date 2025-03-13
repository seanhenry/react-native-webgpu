import {launchArguments} from './launchArguments';

let once = false;

export function examplesCallback(queue?: GPUQueue) {
  const {example} = launchArguments;
  const message = `${example}\n`;
  if (!example || once) {
    return;
  }
  once = true;
  if (!queue) {
    reactNativeWebGPUExperimental.socketCallback(message);
    return;
  }
  queue.onSubmittedWorkDone().then(() => {
    reactNativeWebGPUExperimental.socketCallback(message);
  });
}
