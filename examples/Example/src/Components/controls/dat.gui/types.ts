import {Observable} from '../../../utils/observable';

export type ControlComponentType = {
  folder: {
    isRoot: boolean;
    initialExpanded: boolean;
    children: ControlComponent[];
  };
  function: {
    onPress: () => void;
  };
  slider: {
    min: number;
    max: number;
    step: number;
    initialValue: number;
    onChange: (value: number) => void;
    onFinishChange: () => void;
    observable: Observable<number>;
  };
  number: {
    min?: number;
    max?: number;
    initialValue: number;
    onChange: (value: number) => void;
    onFinishChange: () => void;
    observable: Observable<number>;
  };
  switch: {
    initialValue: boolean;
    onChange: (value: boolean) => void;
    observable: Observable<boolean>;
  };
  options: {
    initialValue: string;
    options: {
      key: string;
      title: string;
      onPress: () => void;
    }[];
    observable: Observable<unknown>;
  };
  string: {
    initialValue: string;
    onChange: (value: string) => void;
    onFinishChange: () => void;
    observable: Observable<string>;
  };
};

interface ControlComponentProps {
  key: string;
  title: string;
  disabled: boolean;
}

// eslint-disable-next-line @typescript-eslint/no-explicit-any
export interface ControlComponent<T extends keyof ControlComponentType = any> {
  type: T;
  props: ControlComponentProps & ControlComponentType[T];
}
