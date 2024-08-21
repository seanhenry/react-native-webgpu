/* eslint-disable @typescript-eslint/no-explicit-any */
import {ControlComponent, ControlComponentType} from './types';
import {getId} from './incrementingId';
import {Observable, observable} from '../../../utils/observable';

export abstract class Controller<
  T = any,
  Component extends keyof ControlComponentType = any,
  Value = any,
> {
  private _key = `controller-${getId('controller')}`;
  private readonly _observable: Observable<Value>;
  private _name: string;
  private _disabled = false;
  private _onChange?: (value: Value) => void;
  private _onFinishChange?: (value: Value) => void;

  constructor(protected object: T, protected property: keyof T) {
    this._name = String(property);
    this._observable = observable<Value>(this.value, {emitDuplicates: true});
  }

  get value(): Value {
    return this.object[this.property] as Value;
  }

  setValue(value: Value, notifyObservers?: boolean) {
    this.object[this.property] = value as any;
    this.change();
    if (notifyObservers ?? true) {
      this.notifyObservers();
    }
    return this;
  }

  getName() {
    return this._name;
  }

  protected get key() {
    return this._key;
  }

  name(name: string) {
    this._name = name;
    return this;
  }

  onChange(onChange: (value: Value) => void) {
    this._onChange = onChange;
    return this;
  }

  onFinishChange(onFinishChange: (value: Value) => void) {
    this._onFinishChange = onFinishChange;
    return this;
  }

  protected change() {
    this._onChange?.call(this, this.value);
    return this;
  }

  protected finishChange() {
    this._onFinishChange?.call(this, this.value);
    return this;
  }

  updateDisplay() {
    this.notifyObservers();
  }

  private notifyObservers() {
    this._observable.post(this.value);
  }

  set disabled(disabled: boolean) {
    this._disabled = disabled;
  }

  get disabled() {
    return this._disabled;
  }

  protected get observable() {
    return this._observable;
  }

  abstract render(): ControlComponent<Component>;
}
