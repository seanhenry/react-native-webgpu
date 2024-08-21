import {Controller} from './Controller';
import {ControlComponent} from './types';

export class SwitchController<T> extends Controller<T, 'switch', boolean> {
  override setValue(value: boolean, notifyObservers: boolean) {
    const result = super.setValue(value, notifyObservers);
    this.finishChange();
    return result;
  }

  render(): ControlComponent<'switch'> {
    return {
      type: 'switch',
      props: {
        key: this.key,
        title: this.getName(),
        disabled: this.disabled,
        initialValue: Boolean(this.value),
        onChange: value => this.setValue(value, false),
        observable: this.observable,
      },
    };
  }
}
