import { EventDispatcher } from 'three';
import { debugLogging } from './debugLogging';

export class TouchEventsAdapter extends EventDispatcher {

  constructor(events) {
    super();
    events.current.onTouchStart = (e) => {
      this.dispatchEvent(this.makeEventProxy('pointerdown', e))
    };
    events.current.onTouchCancel = (e) => {
      this.dispatchEvent(this.makeEventProxy('pointercancel', e))
    };
    events.current.onTouchMove = (e) => {
      this.dispatchEvent(this.makeEventProxy('pointermove', e))
    };
    events.current.onTouchEnd = (e) => {
      this.dispatchEvent(this.makeEventProxy('pointerup', e))
    };
  }

  makeEventProxy(type, event) {
    return new Proxy(event, {
      get(target, propName, receiver) {
        if (propName === 'type') {
          return type;
        } else if (propName === 'pointerId') {
          return event.nativeEvent.identifier;
        } else if (propName === 'pointerType') {
          return 'touch';
        } else if (propName === 'pageX') {
          return event.nativeEvent.pageX;
        } else if (propName === 'pageY') {
          return event.nativeEvent.locationY;
        }
        if (debugLogging) {
          console.log('>> [react-native-webgpu-three] TouchEventsAdapter proxy get:', propName?.description ?? propName);
        }
        return Reflect.get(target, propName, receiver);
      },
    });
  }
}
