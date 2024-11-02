struct Uniforms {
  viewProjectionMat: mat4x4f,
}

struct VertexOut {
  @builtin(position) position: vec4f,
}

@group(0) @binding(0) var <uniform> uniforms: Uniforms;

@vertex
fn v_main(@location(0) position: vec3f) -> VertexOut {
  var out: VertexOut;
  out.position = uniforms.viewProjectionMat * vec4f(position, 1.0);
  return out;
}

@fragment
fn f_cube() -> @location(0) vec4f {
  return vec4f(1.0, 0.0, 0.0, 1.0);
}

@fragment
fn f_outline() -> @location(0) vec4f {
  return vec4f(1.0, 1.0, 1.0, 1.0);
}
