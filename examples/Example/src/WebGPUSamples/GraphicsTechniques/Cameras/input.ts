// Input holds as snapshot of input state
import {type GestureResponderEvent} from 'react-native/Libraries/Types/CoreEventTypes';

export default interface Input {
  // Digital input (e.g keyboard state)
  readonly digital: {
    readonly forward: boolean;
    readonly backward: boolean;
    readonly left: boolean;
    readonly right: boolean;
    readonly up: boolean;
    readonly down: boolean;
  };
  // Analog input (e.g mouse, touchscreen)
  readonly analog: {
    readonly x: number;
    readonly y: number;
    readonly zoom: number;
    readonly touching: boolean;
  };
}

// InputHandler is a function that when called, returns the current Input state.
export type InputHandler = () => Input;

export type InputHandlers = {
  onTouchStart?: (event: GestureResponderEvent) => void;
  onTouchMove?: (event: GestureResponderEvent) => void;
  onTouchEnd?: (event: GestureResponderEvent) => void;
};

// createInputHandler returns an InputHandler by attaching event handlers to the window and canvas.
export function createInputHandler(handlers: InputHandlers): InputHandler {
  const digital = {
    forward: false,
    backward: false,
    left: false,
    right: false,
    up: false,
    down: false,
  };
  const analog = {
    x: 0,
    y: 0,
    zoom: 0,
  };
  let mouseDown = false;

  // const setDigital = (e: KeyboardEvent, value: boolean) => {
  //   switch (e.code) {
  //     case 'KeyW':
  //       digital.forward = value;
  //       e.preventDefault();
  //       e.stopPropagation();
  //       break;
  //     case 'KeyS':
  //       digital.backward = value;
  //       e.preventDefault();
  //       e.stopPropagation();
  //       break;
  //     case 'KeyA':
  //       digital.left = value;
  //       e.preventDefault();
  //       e.stopPropagation();
  //       break;
  //     case 'KeyD':
  //       digital.right = value;
  //       e.preventDefault();
  //       e.stopPropagation();
  //       break;
  //     case 'Space':
  //       digital.up = value;
  //       e.preventDefault();
  //       e.stopPropagation();
  //       break;
  //     case 'ShiftLeft':
  //     case 'ControlLeft':
  //     case 'KeyC':
  //       digital.down = value;
  //       e.preventDefault();
  //       e.stopPropagation();
  //       break;
  //   }
  // };

  // window.addEventListener('keydown', (e) => setDigital(e, true));
  // window.addEventListener('keyup', (e) => setDigital(e, false));

  // canvas.style.touchAction = 'pinch-zoom';
  let lastX = 0;
  let lastY = 0;
  handlers.onTouchStart = e => {
    lastX = e.nativeEvent.pageX;
    lastY = e.nativeEvent.pageY;
    mouseDown = true;
  };
  handlers.onTouchEnd = _e => {
    mouseDown = false;
  };
  handlers.onTouchMove = e => {
    if (mouseDown) {
      analog.x += e.nativeEvent.pageX - lastX;
      analog.y += e.nativeEvent.pageY - lastY;
      lastX = e.nativeEvent.pageX;
      lastY = e.nativeEvent.pageY;
    }
  };
  // canvas.addEventListener(
  //   'wheel',
  //   (e) => {
  //     mouseDown = (e.buttons & 1) !== 0;
  //     if (mouseDown) {
  //       // The scroll value varies substantially between user agents / browsers.
  //       // Just use the sign.
  //       analog.zoom += Math.sign(e.deltaY);
  //       e.preventDefault();
  //       e.stopPropagation();
  //     }
  //   },
  //   { passive: false }
  // );

  return () => {
    const out = {
      digital,
      analog: {
        x: analog.x,
        y: analog.y,
        zoom: analog.zoom,
        touching: mouseDown,
      },
    };
    // Clear the analog values, as these accumulate.
    analog.x = 0;
    analog.y = 0;
    analog.zoom = 0;
    return out;
  };
}
