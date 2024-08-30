@group(0) @binding(0) var texSampler: sampler;
@group(0) @binding(1) var texture: texture_2d<f32>;

struct VertexOutput {
  @builtin(position) position : vec4f,
  @location(0) uv : vec2f,
}

@vertex
fn v_main(
  @builtin(vertex_index) vertexIndex : u32,
) -> VertexOutput {
  var pos = array(
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0)
  );
  var uvs = array(
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0)
  );
  var out: VertexOutput;
  out.position = vec4f(pos[vertexIndex], 0.0, 1.0);
  out.uv = uvs[vertexIndex];
  return out;
}


@fragment
fn f_main(@location(0) uv: vec2f) -> @location(0) vec4f {
  return textureSample(texture, texSampler, uv);
}
