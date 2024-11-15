struct Uniforms {
  ortho: mat4x4f,
}

@group(0) @binding(0) var <uniform> uniforms: Uniforms;

@vertex
fn v_main(
  @location(0) position: vec4f,
) -> @builtin(position) vec4f {
  return uniforms.ortho * position;
}

@fragment
fn f_main() -> @location(0) vec4f {
  return vec4f(0.0, 1.0, 0.0, 1.0);
}
