import {Controller} from './Controller';
import {FunctionController} from './FunctionController';
import {ControlComponent} from './types';
import {observable, ReadonlyObservable} from '../../../utils/observable';
import {SliderController} from './SliderController';
import {SwitchController} from './SwitchController';
import {OptionsController} from './OptionsController';
import {getId} from './incrementingId';
import {StringController} from './StringController';
import {NumberController} from './NumberController';

type GuiParams = {
  parent?: Gui;
  name?: string;
};

export class Gui {
  private _key = `controller-${getId('gui')}`;
  private _items: (Gui | Controller)[] = [];
  private _initialExpanded = false;
  private _observable = observable<ControlComponent<'folder'> | null>(null);
  private _parent?: Gui;
  public name?: string;

  constructor(params?: GuiParams) {
    this.name = params?.name;
    this._parent = params?.parent;
  }

  addFolder(name: string) {
    const gui = new Gui({name, parent: this});
    this._items.push(gui);
    return gui;
  }

  get observable(): ReadonlyObservable<ControlComponent<'folder'> | null> {
    return this._observable;
  }

  open() {
    this._initialExpanded = true;
  }

  close() {
    this._initialExpanded = false;
  }

  updateDisplay() {
    this._items.forEach(item => {
      if ('updateDisplay' in item) {
        item.updateDisplay();
      }
    });
  }

  draw() {
    return this._observable.post(this.render());
  }

  render(): ControlComponent<'folder'> {
    return {
      type: 'folder',
      props: {
        key: this._key,
        title: this.name ?? '',
        disabled: false,
        initialExpanded: this._initialExpanded,
        isRoot: !this._parent,
        children: Object.values(this._items).map(c => c.render()),
      },
    };
  }

  add<T>(
    object: T,
    property: keyof T,
    options: object,
  ): Controller<T, 'options'>;
  add<T>(object: T, property: keyof T): Controller<T>;
  add<T>(
    object: T,
    property: keyof T,
    min: number,
    max: number,
    step?: number,
  ): SliderController<T>;
  add<T>(object: T, property: keyof T, min: number): NumberController<T>;
  add<T>(
    object: T,
    property: keyof T,
    _: undefined,
    max: number,
  ): NumberController<T>;
  add<T>(object: T, property: keyof T, ...args: any[]): Controller<T> {
    let controller;
    const value = object[property];
    if (typeof args[0] === 'object') {
      controller = new OptionsController(object, property, args[0]);
    } else if (typeof value === 'function') {
      controller = new FunctionController(object, property);
    } else if (typeof value === 'number') {
      if (typeof args[0] === 'number' && typeof args[1] === 'number') {
        controller = new SliderController(
          object,
          property,
          args[0],
          args[1],
          args[2],
        );
      } else {
        controller = new NumberController(object, property, args[0], args[1]);
      }
    } else if (typeof value === 'boolean') {
      controller = new SwitchController(object, property);
    } else if (typeof value === 'string') {
      controller = new StringController(object, property);
    } else {
      throw new Error(
        `Input type ${typeof value} not supported. Prop: ${String(property)}`,
      );
    }
    this._items.push(controller);
    return controller;
  }
}
