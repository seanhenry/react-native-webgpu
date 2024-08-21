import {
  calculateStep,
  formatSteppedNumber,
  normalizedBetween,
  lerpWithStep,
} from './sliderUtils';

describe('calculateStep', () => {
  it.each([
    [1, 0],
    [0.01, 0.1],
    [0.01, 0.2],
    [0.001, 0.01],
    [1, 15],
    [10, 150],
  ])('is %s when initial value is %s', (expected, initialValue) => {
    expect(calculateStep(initialValue)).toBe(expected);
  });
});

describe('formatSteppedNumber', () => {
  it.each([
    ['10', 10, 1],
    ['10', 10.00001, 1],
    ['10', 9.99999, 1],
    ['10', 10, 0.1],
    ['0.5', 0.5, 0.25],
    ['0.667', 0.6666666, 0.333],
  ])('is %s when value is %s and step is %s', (expected, value, step) => {
    expect(formatSteppedNumber(value, step)).toBe(expected);
  });
});

describe('normalizedBetween', () => {
  it.each([
    [0, 4, 5, 10],
    [0, 5, 5, 10],
    [0.5, 7.5, 5, 10],
    [1, 10, 5, 10],
    [1, 11, 5, 10],
  ])('is %s when value: %s min: %s max: %s', (expected, value, min, max) => {
    expect(normalizedBetween(value, min, max)).toBe(expected);
  });

  it('is min when max === min', () => {
    expect(normalizedBetween(1, 5, 5)).toBe(5);
  });
});

describe('lerpWithStep', () => {
  it.each([
    [5, -1, 5, 10, 1],
    [5, 0, 5, 10, 1],
    [10, 1, 5, 10, 1],
    [10, 2, 5, 10, 1],
    [7, 0.3, 5, 10, 2],
  ])(
    'is %s when value: %s min: %s max: %s step: %s',
    (expected, value, min, max, step) => {
      expect(lerpWithStep(value, min, max, step)).toBe(expected);
    },
  );

  it('throws when step is 0', () => {
    expect(() => lerpWithStep(1, 5, 5, 0)).toThrow();
  });
});
