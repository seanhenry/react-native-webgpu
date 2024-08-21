import {act, renderHook} from '@testing-library/react-native';

import type {Observable} from './observable';
import {observable, useObservable} from './observable';
import {useRef} from 'react';

jest.useFakeTimers();

describe('observable', () => {
  let ob: Observable<string>;
  beforeEach(() => {
    ob = observable('initial');
  });

  const addObserverWithExpectation = () =>
    ob.add(value => expect(value).toBe('update'));

  it('has initial value', () => {
    expect(ob.value()).toBe('initial');
  });

  it('posts to observers', () => {
    expect.assertions(2);
    addObserverWithExpectation();
    addObserverWithExpectation();
    ob.post('update');
  });

  it('can remove observer twice', () => {
    expect.assertions(1);
    addObserverWithExpectation();
    const remove = addObserverWithExpectation();
    remove();
    remove(); // ensures other observers aren't removed when using api accidentally
    ob.post('update');
  });

  it('does not update when values are equal', () => {
    expect.assertions(0);
    addObserverWithExpectation();
    ob.post('initial');
  });
});

describe('observable when reference type', () => {
  const initial = {initial: {deep: 'value'}};
  const updated = {initial: {deep: 'updated'}};
  let ob: Observable<typeof initial>;
  beforeEach(() => {
    ob = observable(initial);
  });

  it('returns copy of initial value', () => {
    expect(ob.value()).not.toBe(initial);
  });

  it('return copy of updated value', () => {
    ob.post(updated);
    expect(ob.value()).not.toBe(updated);
  });

  it('passes copy of updated value to each observer', () => {
    expect.assertions(2);
    ob.add(v => expect(v).not.toBe(updated));
    ob.add(v => expect(v).not.toBe(updated));
    ob.post(updated);
  });
});

describe('useObservable.useState', () => {
  const useTestHook = () => {
    const {useState, post} = useObservable<{value: string}>({value: 'initial'});
    const state = useState();
    const renders = useRef(0);
    renders.current += 1;
    return {state, post, renders: renders.current};
  };
  const render = () => renderHook(useTestHook);
  let hook: ReturnType<typeof render>;
  beforeEach(() => {
    hook = render();
  });

  it('updates state on change', () => {
    act(() => hook.result.current.post({value: 'newValue'}));
    expect(hook.result.current.state).toEqual({value: 'newValue'});
  });

  it('updates only when state changes', () => {
    hook.result.current.post({value: 'initial'});
    expect(hook.result.current.renders).toBe(1);
  });

  it('updates once when equal, but instance changes', () => {
    act(() => hook.result.current.post({value: 'newValue'}));
    expect(hook.result.current.renders).toBe(2);
    act(() => hook.result.current.post({value: 'newValue'}));
    expect(hook.result.current.renders).toBe(2);
  });
});

describe('useObservable.useStateWithReducer', () => {
  const useTestHook = (reduce: (value: string) => boolean) => {
    const {useStateWithReducer, post} = useObservable<string>('initial');
    const isInitial = useStateWithReducer(reduce);
    const renders = useRef(0);
    renders.current += 1;
    return {isInitial, post, renders: renders.current};
  };
  const render = () =>
    renderHook(useTestHook, {initialProps: value => value === 'initial'});
  let hook: ReturnType<typeof render>;
  beforeEach(() => {
    hook = render();
  });

  it('sets initial state ', () => {
    expect(hook.result.current.isInitial).toBe(true);
  });

  it('updates state', () => {
    act(() => hook.result.current.post('newValue'));
    expect(hook.result.current.isInitial).toBe(false);
  });

  it('updates state only when reducer value changes', () => {
    hook.result.current.post('initial');
    expect(hook.result.current.isInitial).toBe(true);
    expect(hook.result.current.renders).toBe(1);
  });

  it('updates state when reducer function changes', () => {
    hook.rerender(value => value === 'newReducer');
    expect(hook.result.current.isInitial).toBe(false);
    act(() => hook.result.current.post('newReducer'));
    expect(hook.result.current.isInitial).toBe(true);
  });
});
