@vertex
fn v_main(
  @builtin(vertex_index) VertexIndex : u32
) -> @builtin(position) vec4f {
  var pos = array<vec2f, 3>( // triangle is drawn clockwise
    vec2(0.0, 0.5),
    vec2(0.5, -0.5),
    vec2(-0.5, -0.5)
  );

  return vec4f(pos[VertexIndex], 0.0, 1.0);
}

@fragment
fn f_main() -> @location(0) vec4f {
  return vec4f(1.0, 1.0, 0.0, 1.0);
}
