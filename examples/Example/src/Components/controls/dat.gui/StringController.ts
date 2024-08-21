import {Controller} from './Controller';
import {ControlComponent} from './types';

export class StringController<T> extends Controller<T, 'string', string> {
  render(): ControlComponent<'string'> {
    return {
      type: 'string',
      props: {
        key: this.key,
        title: this.getName(),
        disabled: this.disabled,
        initialValue: this.value,
        observable: this.observable,
        onChange: value =>
          this.setValue(
            value,
            true, // true because the TextInput is controlled by this.value
          ),
        onFinishChange: () => this.finishChange(),
      },
    };
  }
}
