import {Controller} from './Controller';
import {ControlComponent} from './types';

export class OptionsController<T> extends Controller<T, 'options', unknown> {
  private options: Record<string, unknown>;
  constructor(object: T, property: keyof T, options: object) {
    super(object, property);
    if (Array.isArray(options)) {
      this.options = {};
      options.forEach(opt => {
        this.options[String(opt)] = opt;
      });
    } else {
      this.options = options as Record<string, unknown>;
    }
  }

  override setValue(value: unknown, notifyObservers?: boolean) {
    const result = super.setValue(value, notifyObservers);
    this.finishChange();
    return result;
  }

  render(): ControlComponent<'options'> {
    return {
      type: 'options',
      props: {
        key: this.key,
        title: this.getName(),
        disabled: this.disabled,
        initialValue: String(this.value),
        options: Object.entries(this.options).map(([key, value]) => ({
          key,
          title: key,
          onPress: () => this.setValue(value, false),
        })),
        observable: this.observable,
      },
    };
  }
}
