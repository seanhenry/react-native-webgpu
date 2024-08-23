import NodeBuilder from 'three/examples/jsm/nodes/core/NodeBuilder.js';
import ParameterNode from 'three/examples/jsm/nodes/core/ParameterNode.js';

// See DEV.md
// Original code is commented out
NodeBuilder.prototype.flowShaderNode = function (shaderNode) {
  const layout = shaderNode.layout;

  let inputs;

  // if ( shaderNode.isArrayInput ) {

  inputs = [];

  for (const input of layout.inputs) {
    inputs.push(new ParameterNode(input.type, input.name));
  }

  // } else {
  //
  // inputs = {};

  for (const input of layout.inputs) {
    inputs[input.name] = new ParameterNode(input.type, input.name);
  }

  // }

  //

  shaderNode.layout = null;

  const callNode = shaderNode.call(inputs);
  const flowData = this.flowStagesNode(callNode, layout.type);

  shaderNode.layout = layout;

  return flowData;
};
