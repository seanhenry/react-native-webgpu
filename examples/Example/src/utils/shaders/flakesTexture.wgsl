struct VertexOutput {
  @builtin(position) position : vec4f,
  @location(0) color : vec3f,
}

const circlePoints: f32 = 10.0;

@vertex
fn v_main(
  @builtin(vertex_index) vertexIndex : u32,
  @location(0) radius: f32,
  @location(1) position: vec2<f32>,
  @location(2) color: vec3<f32>
) -> VertexOutput {
  var step = vertexIndex / 3;
  var remainder = vertexIndex % 3;
  var dAngle = radians(360.0) / circlePoints;
  var pos = vec4(position, 0.0, 1.0);

  if (remainder == 1) {
    var angle = f32(step) * dAngle;
    pos.x += radius * cos(angle);
    pos.y += radius * sin(angle);
  } else if (remainder == 2) {
    var angle = f32(step+1) * dAngle;
    pos.x += radius * cos(angle);
    pos.y += radius * sin(angle);
  }

  var out: VertexOutput;
  out.position = pos;
  out.color = color;
  return out;
}

@fragment
fn f_main(@location(0) color: vec3f) -> @location(0) vec4f {
  return vec4(color, 1.0);
}
