import {Controller} from './Controller';
import {ControlComponent} from './types';

export class NumberController<T> extends Controller<T, 'number', number> {
  constructor(
    object: T,
    property: keyof T,
    private min: unknown,
    private max: unknown,
  ) {
    super(object, property);
  }

  render(): ControlComponent<'number'> {
    return {
      type: 'number',
      props: {
        key: this.key,
        title: this.getName(),
        disabled: this.disabled,
        min: typeof this.min === 'number' ? this.min : undefined,
        max: typeof this.max === 'number' ? this.max : undefined,
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
