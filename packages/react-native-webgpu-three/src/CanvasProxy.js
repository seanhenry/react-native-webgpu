import { debugLogging, makeLoggingProxy } from './debugLogging';

export const makeCanvasProxy = ({payload, eventsAdapter}) => new Proxy({}, {
  get(target, propName, receiver) {
    if (propName === 'width') {
      return payload.context.width;
    } else if (propName === 'height') {
      return payload.context.height;
    } else if (propName === 'getRootNode') {
      return () => {
        return makeLoggingProxy({
          addEventListener: () => {}
        }, ['canvas.getRootNode()']);
      };
    } else if (propName === 'addEventListener') {
      return (...args) => eventsAdapter.addEventListener(...args);
    } else if (propName === 'removeEventListener') {
      return (...args) => eventsAdapter.removeEventListener(...args);
    } else if (propName === 'style') {
      return {};
    } else if (propName === 'setPointerCapture') {
      return () => {};
    } else if (propName === 'releasePointerCapture') {
      return () => {};
    } else if (propName === 'clientHeight') {
      return payload.context.height / payload.context.scale;
    }
    if (debugLogging) {
      console.log(`>> [react-native-webgpu-three] get: canvas.${propName.description ?? propName}`);
    }
  },
})
