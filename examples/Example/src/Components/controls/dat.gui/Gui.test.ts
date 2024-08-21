import {Gui} from './Gui';
import {ControlComponent} from './types';

const mockFn = jest.fn();
const onChange = jest.fn();
const onFinishChange = jest.fn();
const makeConfigObject = () => ({
  func: mockFn,
  slider: 0,
  switch: false,
  opts: 'b',
  numOpts: 1,
  string: 'value',
});
let object = makeConfigObject();

describe('Gui', () => {
  let gui: Gui;
  beforeEach(() => {
    object = makeConfigObject();
    jest.resetAllMocks();
    gui = new Gui();
  });

  describe('function', () => {
    it('adds a function controller', () => {
      gui.add(object, 'func').name('Button');
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'function',
            props: {
              key: expect.any(String),
              title: 'Button',
              disabled: false,
              onPress: expect.any(Function),
            },
          },
        ]),
      );

      childAt(render, 0).props.onPress();
      expect(mockFn).toBeCalled();
    });

    it('function controller sends onChange events', () => {
      gui.add(object, 'func').onChange(onChange).onFinishChange(onFinishChange);
      const render = gui.render();
      childAt(render, 0).props.onPress();
      expect(onChange).toBeCalled();
      expect(onFinishChange).toBeCalled();
    });
  });

  describe('number', () => {
    it('adds a slider controller', () => {
      gui.add(object, 'slider', 0, 10);
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'slider',
            props: {
              key: expect.any(String),
              title: 'slider',
              disabled: false,
              initialValue: 0,
              min: 0,
              max: 10,
              step: 1,
              onChange: expect.any(Function),
              onFinishChange: expect.any(Function),
              observable: expect.any(Object),
            },
          },
        ]),
      );
    });

    it('adds a slider controller with step', () => {
      gui.add(object, 'slider', 0, 10, 2);
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'slider',
            props: {
              key: expect.any(String),
              title: 'slider',
              disabled: false,
              initialValue: 0,
              min: 0,
              max: 10,
              step: 2,
              onChange: expect.any(Function),
              onFinishChange: expect.any(Function),
              observable: expect.any(Object),
            },
          },
        ]),
      );
    });

    it('adds a number controller', () => {
      gui.add(object, 'slider');
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'number',
            props: {
              key: expect.any(String),
              title: 'slider',
              disabled: false,
              initialValue: 0,
              onChange: expect.any(Function),
              onFinishChange: expect.any(Function),
              observable: expect.any(Object),
            },
          },
        ]),
      );
    });

    it('adds a number controller with min', () => {
      gui.add(object, 'slider', 1);
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'number',
            props: {
              key: expect.any(String),
              title: 'slider',
              disabled: false,
              initialValue: 0,
              min: 1,
              onChange: expect.any(Function),
              onFinishChange: expect.any(Function),
              observable: expect.any(Object),
            },
          },
        ]),
      );
    });

    it('adds a number controller with max', () => {
      gui.add(object, 'slider', undefined, 1);
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'number',
            props: {
              key: expect.any(String),
              title: 'slider',
              disabled: false,
              initialValue: 0,
              max: 1,
              onChange: expect.any(Function),
              onFinishChange: expect.any(Function),
              observable: expect.any(Object),
            },
          },
        ]),
      );
    });

    it('calls onChange when slider changes', () => {
      const controller = gui
        .add(object, 'slider', 0, 10)
        .onChange(onChange)
        .onFinishChange(onFinishChange);
      const render = gui.render();
      childAt(render, 0).props.onChange(11);
      expect(onChange).toBeCalledWith(11);
      childAt(render, 0).props.onFinishChange();
      expect(onFinishChange).toBeCalledWith(11);
      expect(controller.value).toBe(11);
    });

    it('calls onChange when number changes', () => {
      const controller = gui
        .add(object, 'slider')
        .onChange(onChange)
        .onFinishChange(onFinishChange);
      const render = gui.render();
      childAt(render, 0).props.onChange(11);
      expect(onChange).toBeCalledWith(11);
      childAt(render, 0).props.onFinishChange();
      expect(onFinishChange).toBeCalledWith(11);
      expect(controller.value).toBe(11);
    });
  });

  describe('boolean', () => {
    it('adds a switch controller', () => {
      gui.add(object, 'switch');
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'switch',
            props: {
              key: expect.any(String),
              title: 'switch',
              disabled: false,
              initialValue: false,
              onChange: expect.any(Function),
              observable: expect.any(Object),
            },
          },
        ]),
      );
    });

    it('calls onChange when switch changes', () => {
      const controller = gui
        .add(object, 'switch')
        .onChange(onChange)
        .onFinishChange(onFinishChange);
      const render = gui.render();
      childAt(render, 0).props.onChange(true);
      expect(onChange).toBeCalledWith(true);
      expect(onFinishChange).toBeCalledWith(true);
      expect(controller.value).toBe(true);
    });
  });

  describe('options', () => {
    it('adds array options', () => {
      gui.add(object, 'opts', ['a', 'b']);
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'options',
            props: {
              key: expect.any(String),
              title: 'opts',
              disabled: false,
              initialValue: 'b',
              observable: expect.any(Object),
              options: [
                {
                  key: 'a',
                  title: 'a',
                  onPress: expect.any(Function),
                },
                {
                  key: 'b',
                  title: 'b',
                  onPress: expect.any(Function),
                },
              ],
            },
          },
        ]),
      );
    });

    it('adds object options', () => {
      gui.add(object, 'opts', {a: 0, b: 1});
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'options',
            props: {
              key: expect.any(String),
              title: 'opts',
              disabled: false,
              initialValue: 'b',
              observable: expect.any(Object),
              options: [
                {
                  key: 'a',
                  title: 'a',
                  onPress: expect.any(Function),
                },
                {
                  key: 'b',
                  title: 'b',
                  onPress: expect.any(Function),
                },
              ],
            },
          },
        ]),
      );
    });

    it('calls onChange when option selected', () => {
      gui
        .add(object, 'numOpts', {a: 0, b: 1})
        .onChange(onChange)
        .onFinishChange(onFinishChange);
      const render = gui.render();
      childAt(render, 0).props.options[0].onPress();
      expect(onChange).toBeCalledWith(0);
      expect(onFinishChange).toBeCalledWith(0);
      expect(object.numOpts).toBe(0);
    });
  });

  describe('string', () => {
    it('adds string', () => {
      gui.add(object, 'string');
      const render = gui.render();
      expect(render).toEqual(
        rootFolder([
          {
            type: 'string',
            props: {
              key: expect.any(String),
              title: 'string',
              disabled: false,
              initialValue: 'value',
              observable: expect.any(Object),
              onChange: expect.any(Function),
              onFinishChange: expect.any(Function),
            },
          },
        ]),
      );
    });

    it('calls onChange when string changed', () => {
      gui
        .add(object, 'string')
        .onChange(onChange)
        .onFinishChange(onFinishChange);
      const render = gui.render();
      childAt(render, 0).props.onChange('new value');
      expect(onChange).toBeCalledWith('new value');
      childAt(render, 0).props.onFinishChange();
      expect(onFinishChange).toBeCalledWith('new value');
    });
  });

  it('calls observable when value changed manually', () => {
    const mockObserver = jest.fn();
    const controller = gui.add(object, 'slider', 0, 1, 0.1);
    const render = gui.render();
    childAt(render, 0).props.observable.add(mockObserver);
    controller.setValue(0.4, false);
    expect(mockObserver).toBeCalledTimes(0);
    controller.setValue(0.4, true);
    expect(mockObserver).toBeCalledWith(0.4);
  });

  it('sets name', () => {
    const controller = gui.add(object, 'string');
    controller.name('NAME');
    const render = gui.render();
    expect(childAt(render, 0).props.title).toBe('NAME');
  });

  it('enables and disables controller', () => {
    const controller = gui.add(object, 'string');
    controller.disabled = true;
    let render = gui.render();
    expect(childAt(render, 0).props.disabled).toBe(true);

    controller.disabled = false;
    render = gui.render();
    expect(childAt(render, 0).props.disabled).toBe(false);
  });

  it('opens and closes gui', () => {
    gui.open();
    expect(gui.render()).toEqual(rootFolder([], true));
    gui.close();
    expect(gui.render()).toEqual(rootFolder([], false));
  });

  it('adds a folder', () => {
    const folder = gui.addFolder('My folder');
    folder.add(object, 'func');
    const render = gui.render();
    expect(render).toEqual(
      rootFolder([
        {
          type: 'folder',
          props: {
            key: expect.any(String),
            title: 'My folder',
            disabled: false,
            isRoot: false,
            initialExpanded: false,
            children: [
              {
                type: 'function',
                props: {
                  key: expect.any(String),
                  title: 'func',
                  disabled: false,
                  onPress: expect.any(Function),
                },
              },
            ],
          },
        },
      ]),
    );
  });
});

const rootFolder = (children: ControlComponent[], initialExpanded = false) => ({
  type: 'folder',
  props: {
    key: expect.any(String),
    title: '',
    disabled: false,
    isRoot: true,
    initialExpanded,
    children,
  },
});

const childAt = (component: ControlComponent, index: number) => {
  return component.props.children[index];
};
