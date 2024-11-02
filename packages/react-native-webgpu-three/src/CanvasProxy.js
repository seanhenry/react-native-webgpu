import {debugLogging, makeLoggingProxy} from './debugLogging';

export const makeCanvasProxy = ({weakContext, eventsAdapter}) =>
  new Proxy(
    {},
    {
      get(target, propName, receiver) {
        if (propName === 'width') {
          return weakContext.deref()?.width ?? 0;
        } else if (propName === 'height') {
          return weakContext.deref()?.height ?? 0;
        } else if (propName === 'getRootNode') {
          return () => {
            return makeLoggingProxy(
              {
                addEventListener: () => {},
              },
              ['canvas.getRootNode()'],
            );
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
          const context = weakContext.deref();
          return context ? context.height / context.scale : 0;
        }
        if (debugLogging) {
          console.log(
            `>> [react-native-webgpu-three] get: canvas.${propName.description ?? propName}`,
          );
        }
      },
    },
  );
