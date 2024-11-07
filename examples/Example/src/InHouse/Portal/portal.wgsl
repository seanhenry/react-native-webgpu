struct Uniforms {
  viewProjectionMat: mat4x4f,
}

struct VertexOut {
  @builtin(position) position: vec4f,
  @location(0) uv: vec2f,
}

@group(0) @binding(0) var <uniform> uniforms: Uniforms;

@vertex
fn v_main(@location(0) position: vec3f, @location(1) uv: vec2f) -> VertexOut {
  var out: VertexOut;
  out.position = uniforms.viewProjectionMat * vec4f(position, 1.0);
  out.uv = uv;
  return out;
}

@group(1) @binding(0) var texSampler: sampler;
@group(1) @binding(1) var tex: texture_2d<f32>;

@fragment
fn f_main(@location(0) uv: vec2f) -> @location(0) vec4f {
  return textureSample(tex, texSampler, uv);
}

@vertex
fn v_portal(@location(0) position: vec3f) -> VertexOut {
  var out: VertexOut;
  out.position = uniforms.viewProjectionMat * vec4f(position, 1.0);
  return out;
}

@fragment
fn f_portal() -> @location(0) vec4f {
  return vec4f(1.0, 0.0, 0.0, 1.0);
}
