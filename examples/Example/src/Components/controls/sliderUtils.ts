export const calculateStep = (initialValue: number) => {
  if (initialValue === 0) {
    return 1;
  } else {
    return (
      Math.pow(10, Math.floor(Math.log(Math.abs(initialValue)) / Math.LN10)) /
      10
    );
  }
};

export const clamp = (value: number, min: number, max: number) => {
  'worklet';
  return Math.max(min, Math.min(max, value));
};

export const formatSteppedNumber = (value: number, step: number) => {
  'worklet';
  let numDecimals = 0;
  const stringStep = `${step}`;
  const i = stringStep.indexOf('.');
  if (i > -1) {
    numDecimals = stringStep.length - i - 1;
  }

  const tenTo = Math.pow(10, numDecimals);
  return `${Math.round(value * tenTo) / tenTo}`;
};

export const normalizedBetween = (value: number, min: number, max: number) => {
  'worklet';
  if (max === min) {
    return min;
  }
  return clamp((value - min) / (max - min), 0, 1);
};

export const lerpWithStep = (
  value: number,
  min: number,
  max: number,
  step: number,
) => {
  'worklet';
  const unnormalized = value * (max - min);
  if (step === 0) {
    throw new Error('Step cannot be zero');
  }
  const stepped = step * Math.round(unnormalized / step);
  return clamp(min + stepped, min, max);
};
