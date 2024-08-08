import ConstNode from 'three/examples/jsm/nodes/core/ConstNode'

ConstNode.prototype.generate = function( builder, output ) {

  const type = this.getNodeType( builder );

  if (type === 'float' && output === 'int') {
    return `${Math.round(parseFloat(this.generateConst( builder )))}`
  }

  return builder.format( this.generateConst( builder ), type, output );

}
