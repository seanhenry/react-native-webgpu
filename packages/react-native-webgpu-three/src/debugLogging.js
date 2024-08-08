export let debugLogging = false

export const enableDebugLogging = (enable) => {
  debugLogging = enable;
}

export const makeLoggingProxy = (target, propNames) => new Proxy(target, {
  apply: (target, thisArg, argArray) => {
    if (debugLogging) {
      console.log(`>> [react-native-webgpu-three] apply: ${propNames.join('.')}(`, ...argArray, ')');
    }
    return Reflect.apply(target, thisArg, argArray);
  },
  get(target, propName, receiver) {
    if (debugLogging) {
      console.log(`>> [react-native-webgpu-three] get: ${propNames.join('.')}.${propName.description ?? propName}`)
    }
    return Reflect.get(target, propName, receiver);
  },
  set(target, propName, value, receiver) {
    if (debugLogging) {
      console.log(`>> [react-native-webgpu-three] set: ${propNames.join('.')}.${propName.description ?? propName} = ${value}`)
    }
    return Reflect.set(target, propName, value, receiver);
  }
})
