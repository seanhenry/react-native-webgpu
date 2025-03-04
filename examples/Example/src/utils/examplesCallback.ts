import {launchArguments} from './launchArguments';

let once = false;

export function examplesCallback(queue?: GPUQueue) {
  const {example} = launchArguments;
  if (!example || once) {
    return;
  }
  once = true;
  if (!queue) {
    reactNativeWebGPUExperimental.socketCallback(example);
    return;
  }
  queue.onSubmittedWorkDone().then(() => {
    reactNativeWebGPUExperimental.socketCallback(example);
  });
}
