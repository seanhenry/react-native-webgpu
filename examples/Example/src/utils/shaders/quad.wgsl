struct VertexOut {
  @builtin(position) position: vec4f,
  @location(0) uvs: vec2f,
}

@vertex
fn v_main(@builtin(vertex_index) index: u32) -> VertexOut {
  var points = array(
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0)
  );
  var uvs = array(
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0)
  );

  var out: VertexOut;
  out.position = vec4f(points[index], 0.0, 1.0);
  out.uvs = uvs[index];
  return out;
}

@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;

@fragment
fn f_main(in: VertexOut) -> @location(0) vec4f {
  return textureSample(myTexture, mySampler, in.uvs);
}
