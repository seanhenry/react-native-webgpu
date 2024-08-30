@group(0) @binding(0) var textureOut: texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(1) var yTexture: texture_2d<f32>;
@group(0) @binding(2) var cbTexture: texture_2d<f32>;
@group(0) @binding(3) var crTexture: texture_2d<f32>;

@group(1) @binding(0) var<uniform> flags: u32;

const kFlipYFlag = 1u;
const kIsInterleavedFlag = 2u;

@compute @workgroup_size(1)
fn main(@builtin(global_invocation_id) grid: vec3u) {
  var flipY = (flags & kFlipYFlag) != 0;
  var isInterleaved = (flags & kIsInterleavedFlag) != 0;
  var ySize = textureDimensions(yTexture);
  var cbSize = textureDimensions(cbTexture);
  var xy = grid.xy;

  if (flipY) {
    xy.y = ySize.y - xy.y;
  }

  var Y = textureLoad(yTexture, xy, 0).x;
  var Cb = 0.0;
  var Cr = 0.0;

  if (isInterleaved) {
    xy.y /= 2u;
    var cb = xy;
    var cr = xy;
    if (xy.x % 2 == 0) { // is even
      cr.x += 1u;
    } else {
      cb.x -= 1u;
    }
    Cb = textureLoad(cbTexture, cb, 0).x - 0.5;
    Cr = textureLoad(cbTexture, cr, 0).x - 0.5;
  } else {
    Cb = textureLoad(cbTexture, xy / 2, 0).x - 0.5;
    Cr = textureLoad(crTexture, xy / 2, 0).x - 0.5;
  }

  var r = clamp((Y + 1.370705 * Cr), 0.0, 1.0);
  var g = clamp((Y - (0.698001 * Cr) - (0.337633f * Cb)), 0.0, 1.0);
  var b = clamp((Y + 1.732446 * Cb), 0.0, 1.0);

  var color = vec4f(srgbToLinear(r), srgbToLinear(g), srgbToLinear(b), 1.0);
  textureStore(textureOut, grid.xy, color);
}

fn srgbToLinear(c: f32) -> f32 {
  if (c > 0.04045) {
    return pow((c + 0.055) / 1.055, 2.4);
  } else {
    return c / 12.92;
  }
}
