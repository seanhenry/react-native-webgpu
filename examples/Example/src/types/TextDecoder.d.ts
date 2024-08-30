type TypedArray =
  | Int8Array
  | Uint8Array
  | Uint8ClampedArray
  | Int16Array
  | Uint16Array
  | Int32Array
  | Uint32Array
  | Float16Array
  | Float32Array
  | Float64Array
  | BigInt64Array
  | BigUint64Array;

interface TextDecoderInterface {
  decode(buffer: ArrayBuffer | TypedArray | DataView): string;
}

declare var TextDecoder: {
  prototype: TextDecoderInterface;
  new (encoding: string): TextDecoderInterface;
};
