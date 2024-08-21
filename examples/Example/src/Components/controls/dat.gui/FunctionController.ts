import {Controller} from './Controller';
import {ControlComponent} from './types';

type Fn = () => void;

export class FunctionController<T> extends Controller<T, 'function', Fn> {
  constructor(object: T, property: keyof T, label?: string) {
    super(object, property);
    this.name(label ?? String(property));
  }

  fire() {
    this.change();
    this.value.call(this.object);
    this.finishChange();
  }

  override render(): ControlComponent<'function'> {
    return {
      type: 'function',
      props: {
        key: this.key,
        title: this.getName(),
        disabled: this.disabled,
        onPress: () => this.fire(),
      },
    };
  }
}
