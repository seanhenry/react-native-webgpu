import {launchArguments} from './launchArguments';

let once = false;

export function examplesCallback(queue?: GPUQueue) {
  const {example, callbackid} = launchArguments;
  if (!example || !callbackid || once) {
    return;
  }
  once = true;
  const message = `${example}-${callbackid}`;
  if (!queue) {
    reactNativeWebGPUExperimental.socketCallback(message);
    return;
  }
  queue.onSubmittedWorkDone().then(() => {
    reactNativeWebGPUExperimental.socketCallback(message);
  });
}
