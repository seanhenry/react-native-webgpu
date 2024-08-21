import {Controller} from './Controller';
import {ControlComponent} from './types';
import {calculateStep} from '../sliderUtils';

export class SliderController<T> extends Controller<T, 'slider', number> {
  constructor(
    object: T,
    property: keyof T,
    private min: number,
    private max: number,
    private _step: unknown,
  ) {
    super(object, property);
  }

  render(): ControlComponent<'slider'> {
    return {
      type: 'slider',
      props: {
        key: this.key,
        title: this.getName(),
        disabled: this.disabled,
        min: this.min,
        max: this.max,
        step:
          typeof this._step === 'number'
            ? this._step
            : calculateStep(this.value),
        initialValue: this.value,
        observable: this.observable,
        onChange: value => this.setValue(value, false),
        onFinishChange: () => this.finishChange(),
      },
    };
  }

  step(step: number) {
    this._step = step;
    return this;
  }
}
