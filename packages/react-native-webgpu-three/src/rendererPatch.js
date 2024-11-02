import WebGPURenderer from 'three/examples/jsm/renderers/webgpu/WebGPURenderer';

const superDispose = WebGPURenderer.prototype.dispose;
WebGPURenderer.prototype.dispose = function () {
  superDispose.call(this);
  this.backend.colorBuffer = null;
  if (this.backend.textureUtils) {
    this.backend.textureUtils.colorBuffer = null;
  }
  this.backend.defaultRenderPassdescriptor = null;
  this.backend.device = null;
  this.backend.parameters = null;
  this.backend.context = null;
  this.backend.data = null;
};
