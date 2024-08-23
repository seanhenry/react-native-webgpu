import { EventDispatcher } from "three";

export class Image extends EventDispatcher {
  constructor() {
    super();
    this._threeImageBitmap = null;
  }
  set src(src) {
    reactNativeWebGPU
      .createImageBitmap({ uri: src })
      .then((image) => {
        this._threeImageBitmap = image;
        this.dispatchEvent({ type: "load" });
      })
      .catch(() => this.dispatchEvent({ type: "error" }));
  }

  get data() {
    return this._threeImageBitmap;
  }

  get width() {
    return this._threeImageBitmap?.width;
  }

  get height() {
    return this._threeImageBitmap?.height;
  }
}
