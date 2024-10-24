import {useState, useEffect, useRef} from 'react';
import {cloneDeep, isEqual} from 'lodash';

export type Observer<T> = (key: T) => void;
export type Unsubscribe = () => void;
export type Reducer<T, U> = (value: T) => U;

export interface ReadonlyObservable<T> {
  add(observer: Observer<T>): Unsubscribe;
  destroy(): void;
  value(): T;
  useState(): T;
  useStateWithReducer<U>(reduce: Reducer<T, U>): U;
}

export interface Observable<T> extends ReadonlyObservable<T> {
  post(item: T): void;
}

type Options = {
  emitDuplicates: boolean;
};

export const useObservable = <T>(defaultValue: T) =>
  useRef(observable<T>(defaultValue)).current;

export const observable = <T>(
  initialValue: T,
  options?: Options,
): Observable<T> => new ObservableImpl<T>(initialValue, options);

export class ObservableImpl<T> implements Observable<T> {
  add = this._add.bind(this);

  value = this._getValue.bind(this);

  private observers: Observer<T>[] = [];

  private _value: T;

  constructor(initialValue: T, private options?: Options) {
    this._value = cloneDeep(initialValue);
  }

  protected _getValue() {
    return this._value;
  }

  protected _add(observer: Observer<T>): Unsubscribe {
    this.observers.push(observer);
    return () => {
      const index = this.observers.indexOf(observer);
      if (index !== -1) {
        this.observers.splice(index, 1);
      }
    };
  }

  destroy = () => {
    this.observers = [];
    if (this._value) {
      // @ts-expect-error using this after destroy is undefined behaviour
      delete this._value;
    }
  };

  post = (item: T) => {
    if (!this.options?.emitDuplicates && isEqual(item, this._value)) {
      return;
    }
    this._value = cloneDeep(item);
    this.notify();
  };

  useState = (): T => {
    const [state, setState] = useState(this._value);
    useEffect(() => this.add(setState), []);
    return state;
  };

  useStateWithReducer = <U>(reduce: Reducer<T, U>): U => {
    const [state, setState] = useState<U>(reduce(this._value));
    useEffect(() => {
      setState(reduce(this._value));
      return this.add(v => setState(reduce(v)));
    }, [reduce]);
    return state;
  };

  notify = () => {
    this.observers.forEach(l => l(cloneDeep(this.value())));
  };
}
