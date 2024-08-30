#include "ConstantConversion.h"

#include <unordered_map>

#include "WGPUJsiUtils.h"

const char *WGPUTextureFormatToString(WGPUTextureFormat format) {
  static std::unordered_map<WGPUTextureFormat, const char *> map;
  if (map.size() == 0) {
    map[WGPUTextureFormat_R8Unorm] = "r8unorm";
    map[WGPUTextureFormat_R8Snorm] = "r8snorm";
    map[WGPUTextureFormat_R8Uint] = "r8uint";
    map[WGPUTextureFormat_R8Sint] = "r8sint";
    map[WGPUTextureFormat_R16Uint] = "r16uint";
    map[WGPUTextureFormat_R16Sint] = "r16sint";
    map[WGPUTextureFormat_R16Float] = "r16float";
    map[WGPUTextureFormat_RG8Unorm] = "rg8unorm";
    map[WGPUTextureFormat_RG8Snorm] = "rg8snorm";
    map[WGPUTextureFormat_RG8Uint] = "rg8uint";
    map[WGPUTextureFormat_RG8Sint] = "rg8sint";
    map[WGPUTextureFormat_R32Float] = "r32float";
    map[WGPUTextureFormat_R32Uint] = "r32uint";
    map[WGPUTextureFormat_R32Sint] = "r32sint";
    map[WGPUTextureFormat_RG16Uint] = "rg16uint";
    map[WGPUTextureFormat_RG16Sint] = "rg16sint";
    map[WGPUTextureFormat_RG16Float] = "rg16float";
    map[WGPUTextureFormat_RGBA8Unorm] = "rgba8unorm";
    map[WGPUTextureFormat_RGBA8UnormSrgb] = "rgba8unorm-srgb";
    map[WGPUTextureFormat_RGBA8Snorm] = "rgba8snorm";
    map[WGPUTextureFormat_RGBA8Uint] = "rgba8uint";
    map[WGPUTextureFormat_RGBA8Sint] = "rgba8sint";
    map[WGPUTextureFormat_BGRA8Unorm] = "bgra8unorm";
    map[WGPUTextureFormat_BGRA8UnormSrgb] = "bgra8unorm-srgb";
    map[WGPUTextureFormat_RGB10A2Uint] = "rgb10a2uint";
    map[WGPUTextureFormat_RGB10A2Unorm] = "rgb10a2unorm";
    map[WGPUTextureFormat_RG11B10Ufloat] = "rg11b10ufloat";
    map[WGPUTextureFormat_RGB9E5Ufloat] = "rgb9e5ufloat";
    map[WGPUTextureFormat_RG32Float] = "rg32float";
    map[WGPUTextureFormat_RG32Uint] = "rg32uint";
    map[WGPUTextureFormat_RG32Sint] = "rg32sint";
    map[WGPUTextureFormat_RGBA16Uint] = "rgba16uint";
    map[WGPUTextureFormat_RGBA16Sint] = "rgba16sint";
    map[WGPUTextureFormat_RGBA16Float] = "rgba16float";
    map[WGPUTextureFormat_RGBA32Float] = "rgba32float";
    map[WGPUTextureFormat_RGBA32Uint] = "rgba32uint";
    map[WGPUTextureFormat_RGBA32Sint] = "rgba32sint";
    map[WGPUTextureFormat_Stencil8] = "stencil8";
    map[WGPUTextureFormat_Depth16Unorm] = "depth16unorm";
    map[WGPUTextureFormat_Depth24Plus] = "depth24plus";
    map[WGPUTextureFormat_Depth24PlusStencil8] = "depth24plus-stencil8";
    map[WGPUTextureFormat_Depth32Float] = "depth32float";
    map[WGPUTextureFormat_Depth32FloatStencil8] = "depth32float-stencil8";
    map[WGPUTextureFormat_BC1RGBAUnorm] = "bc1-rgba-unorm";
    map[WGPUTextureFormat_BC1RGBAUnormSrgb] = "bc1-rgba-unorm-srgb";
    map[WGPUTextureFormat_BC2RGBAUnorm] = "bc2-rgba-unorm";
    map[WGPUTextureFormat_BC2RGBAUnormSrgb] = "bc2-rgba-unorm-srgb";
    map[WGPUTextureFormat_BC3RGBAUnorm] = "bc3-rgba-unorm";
    map[WGPUTextureFormat_BC3RGBAUnormSrgb] = "bc3-rgba-unorm-srgb";
    map[WGPUTextureFormat_BC4RUnorm] = "bc4-r-unorm";
    map[WGPUTextureFormat_BC4RSnorm] = "bc4-r-snorm";
    map[WGPUTextureFormat_BC5RGUnorm] = "bc5-rg-unorm";
    map[WGPUTextureFormat_BC5RGSnorm] = "bc5-rg-snorm";
    map[WGPUTextureFormat_BC6HRGBUfloat] = "bc6h-rgb-ufloat";
    map[WGPUTextureFormat_BC6HRGBFloat] = "bc6h-rgb-float";
    map[WGPUTextureFormat_BC7RGBAUnorm] = "bc7-rgba-unorm";
    map[WGPUTextureFormat_BC7RGBAUnormSrgb] = "bc7-rgba-unorm-srgb";
    map[WGPUTextureFormat_ETC2RGB8Unorm] = "etc2-rgb8unorm";
    map[WGPUTextureFormat_ETC2RGB8UnormSrgb] = "etc2-rgb8unorm-srgb";
    map[WGPUTextureFormat_ETC2RGB8A1Unorm] = "etc2-rgb8a1unorm";
    map[WGPUTextureFormat_ETC2RGB8A1UnormSrgb] = "etc2-rgb8a1unorm-srgb";
    map[WGPUTextureFormat_ETC2RGBA8Unorm] = "etc2-rgba8unorm";
    map[WGPUTextureFormat_ETC2RGBA8UnormSrgb] = "etc2-rgba8unorm-srgb";
    map[WGPUTextureFormat_EACR11Unorm] = "eac-r11unorm";
    map[WGPUTextureFormat_EACR11Snorm] = "eac-r11snorm";
    map[WGPUTextureFormat_EACRG11Unorm] = "eac-rg11unorm";
    map[WGPUTextureFormat_EACRG11Snorm] = "eac-rg11snorm";
    map[WGPUTextureFormat_ASTC4x4Unorm] = "astc-4x4-unorm";
    map[WGPUTextureFormat_ASTC4x4UnormSrgb] = "astc-4x4-unorm-srgb";
    map[WGPUTextureFormat_ASTC5x4Unorm] = "astc-5x4-unorm";
    map[WGPUTextureFormat_ASTC5x4UnormSrgb] = "astc-5x4-unorm-srgb";
    map[WGPUTextureFormat_ASTC5x5Unorm] = "astc-5x5-unorm";
    map[WGPUTextureFormat_ASTC5x5UnormSrgb] = "astc-5x5-unorm-srgb";
    map[WGPUTextureFormat_ASTC6x5Unorm] = "astc-6x5-unorm";
    map[WGPUTextureFormat_ASTC6x5UnormSrgb] = "astc-6x5-unorm-srgb";
    map[WGPUTextureFormat_ASTC6x6Unorm] = "astc-6x6-unorm";
    map[WGPUTextureFormat_ASTC6x6UnormSrgb] = "astc-6x6-unorm-srgb";
    map[WGPUTextureFormat_ASTC8x5Unorm] = "astc-8x5-unorm";
    map[WGPUTextureFormat_ASTC8x5UnormSrgb] = "astc-8x5-unorm-srgb";
    map[WGPUTextureFormat_ASTC8x6Unorm] = "astc-8x6-unorm";
    map[WGPUTextureFormat_ASTC8x6UnormSrgb] = "astc-8x6-unorm-srgb";
    map[WGPUTextureFormat_ASTC8x8Unorm] = "astc-8x8-unorm";
    map[WGPUTextureFormat_ASTC8x8UnormSrgb] = "astc-8x8-unorm-srgb";
    map[WGPUTextureFormat_ASTC10x5Unorm] = "astc-10x5-unorm";
    map[WGPUTextureFormat_ASTC10x5UnormSrgb] = "astc-10x5-unorm-srgb";
    map[WGPUTextureFormat_ASTC10x6Unorm] = "astc-10x6-unorm";
    map[WGPUTextureFormat_ASTC10x6UnormSrgb] = "astc-10x6-unorm-srgb";
    map[WGPUTextureFormat_ASTC10x8Unorm] = "astc-10x8-unorm";
    map[WGPUTextureFormat_ASTC10x8UnormSrgb] = "astc-10x8-unorm-srgb";
    map[WGPUTextureFormat_ASTC10x10Unorm] = "astc-10x10-unorm";
    map[WGPUTextureFormat_ASTC10x10UnormSrgb] = "astc-10x10-unorm-srgb";
    map[WGPUTextureFormat_ASTC12x10Unorm] = "astc-12x10-unorm";
    map[WGPUTextureFormat_ASTC12x10UnormSrgb] = "astc-12x10-unorm-srgb";
    map[WGPUTextureFormat_ASTC12x12Unorm] = "astc-12x12-unorm";
    map[WGPUTextureFormat_ASTC12x12UnormSrgb] = "astc-12x12-unorm-srgb";
  }
  return map.at(format);
}

WGPUTextureFormat StringToWGPUTextureFormat(const std::string &format) {
  static std::unordered_map<std::string, WGPUTextureFormat> map;
  if (map.empty()) {
    map["r8unorm"] = WGPUTextureFormat_R8Unorm;
    map["r8snorm"] = WGPUTextureFormat_R8Snorm;
    map["r8uint"] = WGPUTextureFormat_R8Uint;
    map["r8sint"] = WGPUTextureFormat_R8Sint;
    map["r16uint"] = WGPUTextureFormat_R16Uint;
    map["r16sint"] = WGPUTextureFormat_R16Sint;
    map["r16float"] = WGPUTextureFormat_R16Float;
    map["rg8unorm"] = WGPUTextureFormat_RG8Unorm;
    map["rg8snorm"] = WGPUTextureFormat_RG8Snorm;
    map["rg8uint"] = WGPUTextureFormat_RG8Uint;
    map["rg8sint"] = WGPUTextureFormat_RG8Sint;
    map["r32float"] = WGPUTextureFormat_R32Float;
    map["r32uint"] = WGPUTextureFormat_R32Uint;
    map["r32sint"] = WGPUTextureFormat_R32Sint;
    map["rg16uint"] = WGPUTextureFormat_RG16Uint;
    map["rg16sint"] = WGPUTextureFormat_RG16Sint;
    map["rg16float"] = WGPUTextureFormat_RG16Float;
    map["rgba8unorm"] = WGPUTextureFormat_RGBA8Unorm;
    map["rgba8unorm-srgb"] = WGPUTextureFormat_RGBA8UnormSrgb;
    map["rgba8snorm"] = WGPUTextureFormat_RGBA8Snorm;
    map["rgba8uint"] = WGPUTextureFormat_RGBA8Uint;
    map["rgba8sint"] = WGPUTextureFormat_RGBA8Sint;
    map["bgra8unorm"] = WGPUTextureFormat_BGRA8Unorm;
    map["bgra8unorm-srgb"] = WGPUTextureFormat_BGRA8UnormSrgb;
    map["rgb10a2uint"] = WGPUTextureFormat_RGB10A2Uint;
    map["rgb10a2unorm"] = WGPUTextureFormat_RGB10A2Unorm;
    map["rg11b10ufloat"] = WGPUTextureFormat_RG11B10Ufloat;
    map["rgb9e5ufloat"] = WGPUTextureFormat_RGB9E5Ufloat;
    map["rg32float"] = WGPUTextureFormat_RG32Float;
    map["rg32uint"] = WGPUTextureFormat_RG32Uint;
    map["rg32sint"] = WGPUTextureFormat_RG32Sint;
    map["rgba16uint"] = WGPUTextureFormat_RGBA16Uint;
    map["rgba16sint"] = WGPUTextureFormat_RGBA16Sint;
    map["rgba16float"] = WGPUTextureFormat_RGBA16Float;
    map["rgba32float"] = WGPUTextureFormat_RGBA32Float;
    map["rgba32uint"] = WGPUTextureFormat_RGBA32Uint;
    map["rgba32sint"] = WGPUTextureFormat_RGBA32Sint;
    map["stencil8"] = WGPUTextureFormat_Stencil8;
    map["depth16unorm"] = WGPUTextureFormat_Depth16Unorm;
    map["depth24plus"] = WGPUTextureFormat_Depth24Plus;
    map["depth24plus-stencil8"] = WGPUTextureFormat_Depth24PlusStencil8;
    map["depth32float"] = WGPUTextureFormat_Depth32Float;
    map["depth32float-stencil8"] = WGPUTextureFormat_Depth32FloatStencil8;
    map["bc1-rgba-unorm"] = WGPUTextureFormat_BC1RGBAUnorm;
    map["bc1-rgba-unorm-srgb"] = WGPUTextureFormat_BC1RGBAUnormSrgb;
    map["bc2-rgba-unorm"] = WGPUTextureFormat_BC2RGBAUnorm;
    map["bc2-rgba-unorm-srgb"] = WGPUTextureFormat_BC2RGBAUnormSrgb;
    map["bc3-rgba-unorm"] = WGPUTextureFormat_BC3RGBAUnorm;
    map["bc3-rgba-unorm-srgb"] = WGPUTextureFormat_BC3RGBAUnormSrgb;
    map["bc4-r-unorm"] = WGPUTextureFormat_BC4RUnorm;
    map["bc4-r-snorm"] = WGPUTextureFormat_BC4RSnorm;
    map["bc5-rg-unorm"] = WGPUTextureFormat_BC5RGUnorm;
    map["bc5-rg-snorm"] = WGPUTextureFormat_BC5RGSnorm;
    map["bc6h-rgb-ufloat"] = WGPUTextureFormat_BC6HRGBUfloat;
    map["bc6h-rgb-float"] = WGPUTextureFormat_BC6HRGBFloat;
    map["bc7-rgba-unorm"] = WGPUTextureFormat_BC7RGBAUnorm;
    map["bc7-rgba-unorm-srgb"] = WGPUTextureFormat_BC7RGBAUnormSrgb;
    map["etc2-rgb8unorm"] = WGPUTextureFormat_ETC2RGB8Unorm;
    map["etc2-rgb8unorm-srgb"] = WGPUTextureFormat_ETC2RGB8UnormSrgb;
    map["etc2-rgb8a1unorm"] = WGPUTextureFormat_ETC2RGB8A1Unorm;
    map["etc2-rgb8a1unorm-srgb"] = WGPUTextureFormat_ETC2RGB8A1UnormSrgb;
    map["etc2-rgba8unorm"] = WGPUTextureFormat_ETC2RGBA8Unorm;
    map["etc2-rgba8unorm-srgb"] = WGPUTextureFormat_ETC2RGBA8UnormSrgb;
    map["eac-r11unorm"] = WGPUTextureFormat_EACR11Unorm;
    map["eac-r11snorm"] = WGPUTextureFormat_EACR11Snorm;
    map["eac-rg11unorm"] = WGPUTextureFormat_EACRG11Unorm;
    map["eac-rg11snorm"] = WGPUTextureFormat_EACRG11Snorm;
    map["astc-4x4-unorm"] = WGPUTextureFormat_ASTC4x4Unorm;
    map["astc-4x4-unorm-srgb"] = WGPUTextureFormat_ASTC4x4UnormSrgb;
    map["astc-5x4-unorm"] = WGPUTextureFormat_ASTC5x4Unorm;
    map["astc-5x4-unorm-srgb"] = WGPUTextureFormat_ASTC5x4UnormSrgb;
    map["astc-5x5-unorm"] = WGPUTextureFormat_ASTC5x5Unorm;
    map["astc-5x5-unorm-srgb"] = WGPUTextureFormat_ASTC5x5UnormSrgb;
    map["astc-6x5-unorm"] = WGPUTextureFormat_ASTC6x5Unorm;
    map["astc-6x5-unorm-srgb"] = WGPUTextureFormat_ASTC6x5UnormSrgb;
    map["astc-6x6-unorm"] = WGPUTextureFormat_ASTC6x6Unorm;
    map["astc-6x6-unorm-srgb"] = WGPUTextureFormat_ASTC6x6UnormSrgb;
    map["astc-8x5-unorm"] = WGPUTextureFormat_ASTC8x5Unorm;
    map["astc-8x5-unorm-srgb"] = WGPUTextureFormat_ASTC8x5UnormSrgb;
    map["astc-8x6-unorm"] = WGPUTextureFormat_ASTC8x6Unorm;
    map["astc-8x6-unorm-srgb"] = WGPUTextureFormat_ASTC8x6UnormSrgb;
    map["astc-8x8-unorm"] = WGPUTextureFormat_ASTC8x8Unorm;
    map["astc-8x8-unorm-srgb"] = WGPUTextureFormat_ASTC8x8UnormSrgb;
    map["astc-10x5-unorm"] = WGPUTextureFormat_ASTC10x5Unorm;
    map["astc-10x5-unorm-srgb"] = WGPUTextureFormat_ASTC10x5UnormSrgb;
    map["astc-10x6-unorm"] = WGPUTextureFormat_ASTC10x6Unorm;
    map["astc-10x6-unorm-srgb"] = WGPUTextureFormat_ASTC10x6UnormSrgb;
    map["astc-10x8-unorm"] = WGPUTextureFormat_ASTC10x8Unorm;
    map["astc-10x8-unorm-srgb"] = WGPUTextureFormat_ASTC10x8UnormSrgb;
    map["astc-10x10-unorm"] = WGPUTextureFormat_ASTC10x10Unorm;
    map["astc-10x10-unorm-srgb"] = WGPUTextureFormat_ASTC10x10UnormSrgb;
    map["astc-12x10-unorm"] = WGPUTextureFormat_ASTC12x10Unorm;
    map["astc-12x10-unorm-srgb"] = WGPUTextureFormat_ASTC12x10UnormSrgb;
    map["astc-12x12-unorm"] = WGPUTextureFormat_ASTC12x12Unorm;
    map["astc-12x12-unorm-srgb"] = WGPUTextureFormat_ASTC12x12UnormSrgb;
    map["undefined"] = WGPUTextureFormat_Undefined;
  }
  if (map.find(format) == map.end()) {
    throw JSINativeException("Unsupported texture format: " + format);
  }
  return map.at(format);
}

const char *WGPUCompositeAlphaModeToString(WGPUCompositeAlphaMode alphaMode) {
  switch (alphaMode) {
    case WGPUCompositeAlphaMode_Premultiplied:
      return "premultiplied";
    case WGPUCompositeAlphaMode_Opaque:
      return "opaque";
    case WGPUCompositeAlphaMode_Inherit:
      return "inherit";
    case WGPUCompositeAlphaMode_Unpremultiplied:
      return "unpremultiplied";
    case WGPUCompositeAlphaMode_Auto:
      return "auto";
    default:
      return "auto";
  }
}

WGPUCompositeAlphaMode StringToWGPUCompositeAlphaMode(const std::string &alphaMode) {
  if (alphaMode == "premultiplied") {
    return WGPUCompositeAlphaMode_Premultiplied;
  } else if (alphaMode == "opaque") {
    return WGPUCompositeAlphaMode_Opaque;
  } else if (alphaMode == "inherit") {
    return WGPUCompositeAlphaMode_Inherit;
  } else if (alphaMode == "unpremultiplied") {
    return WGPUCompositeAlphaMode_Unpremultiplied;
  } else if (alphaMode == "auto") {
    return WGPUCompositeAlphaMode_Auto;
  }
  return WGPUCompositeAlphaMode_Auto;
}

WGPUPrimitiveTopology StringToWGPUPrimitiveTopology(const std::string &topology) {
  if (topology == "point-list") {
    return WGPUPrimitiveTopology_PointList;
  }
  if (topology == "line-list") {
    return WGPUPrimitiveTopology_LineList;
  }
  if (topology == "line-strip") {
    return WGPUPrimitiveTopology_LineStrip;
  }
  if (topology == "triangle-list") {
    return WGPUPrimitiveTopology_TriangleList;
  }
  if (topology == "triangle-strip") {
    return WGPUPrimitiveTopology_TriangleStrip;
  }
  return WGPUPrimitiveTopology_PointList;
}

WGPULoadOp StringToWGPULoadOp(const std::string &str) {
  if (str == "clear") {
    return WGPULoadOp_Clear;
  }
  if (str == "load") {
    return WGPULoadOp_Load;
  }
  return WGPULoadOp_Undefined;
}

WGPUStoreOp StringToWGPUStoreOp(const std::string &str) {
  if (str == "store") {
    return WGPUStoreOp_Store;
  }
  if (str == "discard") {
    return WGPUStoreOp_Discard;
  }
  return WGPUStoreOp_Undefined;
}

WGPUVertexFormat StringToWGPUVertexFormat(const std::string &str) {
  static std::unordered_map<std::string, WGPUVertexFormat> map;
  if (map.empty()) {
    map["uint8x2"] = WGPUVertexFormat_Uint8x2;
    map["uint8x4"] = WGPUVertexFormat_Uint8x4;
    map["sint8x2"] = WGPUVertexFormat_Sint8x2;
    map["sint8x4"] = WGPUVertexFormat_Sint8x4;
    map["unorm8x2"] = WGPUVertexFormat_Unorm8x2;
    map["unorm8x4"] = WGPUVertexFormat_Unorm8x4;
    map["snorm8x2"] = WGPUVertexFormat_Snorm8x2;
    map["snorm8x4"] = WGPUVertexFormat_Snorm8x4;
    map["uint16x2"] = WGPUVertexFormat_Uint16x2;
    map["uint16x4"] = WGPUVertexFormat_Uint16x4;
    map["sint16x2"] = WGPUVertexFormat_Sint16x2;
    map["sint16x4"] = WGPUVertexFormat_Sint16x4;
    map["unorm16x2"] = WGPUVertexFormat_Unorm16x2;
    map["unorm16x4"] = WGPUVertexFormat_Unorm16x4;
    map["snorm16x2"] = WGPUVertexFormat_Snorm16x2;
    map["snorm16x4"] = WGPUVertexFormat_Snorm16x4;
    map["float16x2"] = WGPUVertexFormat_Float16x2;
    map["float16x4"] = WGPUVertexFormat_Float16x4;
    map["float32"] = WGPUVertexFormat_Float32;
    map["float32x2"] = WGPUVertexFormat_Float32x2;
    map["float32x3"] = WGPUVertexFormat_Float32x3;
    map["float32x4"] = WGPUVertexFormat_Float32x4;
    map["uint32"] = WGPUVertexFormat_Uint32;
    map["uint32x2"] = WGPUVertexFormat_Uint32x2;
    map["uint32x3"] = WGPUVertexFormat_Uint32x3;
    map["uint32x4"] = WGPUVertexFormat_Uint32x4;
    map["sint32"] = WGPUVertexFormat_Sint32;
    map["sint32x2"] = WGPUVertexFormat_Sint32x2;
    map["sint32x3"] = WGPUVertexFormat_Sint32x3;
    map["sint32x4"] = WGPUVertexFormat_Sint32x4;
  }
  if (map.find(str) == map.end()) {
    throw JSINativeException("Unsupported vertex format: " + str);
  }
  return map[str];
}

WGPUVertexStepMode StringToWGPUVertexStepMode(const std::string &str) {
  if (str == "vertex") {
    return WGPUVertexStepMode_Vertex;
  }
  if (str == "instance") {
    return WGPUVertexStepMode_Instance;
  }
  return WGPUVertexStepMode_Vertex;
}

WGPUCullMode StringToWGPUCullMode(const std::string &str) {
  if (str == "none") {
    return WGPUCullMode_None;
  }
  if (str == "front") {
    return WGPUCullMode_Front;
  }
  if (str == "back") {
    return WGPUCullMode_Back;
  }
  return WGPUCullMode_None;
}

WGPUCompareFunction StringToWGPUCompareFunction(const std::string &str) {
  if (str == "never") {
    return WGPUCompareFunction_Never;
  }
  if (str == "less") {
    return WGPUCompareFunction_Less;
  }
  if (str == "equal") {
    return WGPUCompareFunction_Equal;
  }
  if (str == "less-equal") {
    return WGPUCompareFunction_LessEqual;
  }
  if (str == "greater") {
    return WGPUCompareFunction_Greater;
  }
  if (str == "not-equal") {
    return WGPUCompareFunction_NotEqual;
  }
  if (str == "greater-equal") {
    return WGPUCompareFunction_GreaterEqual;
  }
  if (str == "always") {
    return WGPUCompareFunction_Always;
  }
  return WGPUCompareFunction_Undefined;
}

WGPUFilterMode StringToWGPUFilterMode(const std::string &str) {
  if (str == "nearest") {
    return WGPUFilterMode_Nearest;
  }
  if (str == "linear") {
    return WGPUFilterMode_Linear;
  }
  return WGPUFilterMode_Nearest;
}

WGPUMipmapFilterMode StringToWGPUMipmapFilterMode(const std::string &str) {
  if (str == "nearest") {
    return WGPUMipmapFilterMode_Nearest;
  }
  if (str == "linear") {
    return WGPUMipmapFilterMode_Linear;
  }
  return WGPUMipmapFilterMode_Nearest;
}

WGPUTextureDimension StringToWGPUTextureDimension(const std::string &str) {
  if (str == "1d") {
    return WGPUTextureDimension_1D;
  }
  if (str == "2d") {
    return WGPUTextureDimension_2D;
  }
  if (str == "3d") {
    return WGPUTextureDimension_3D;
  }
  return WGPUTextureDimension_2D;
}

const char *WGPUTextureDimensionToString(WGPUTextureDimension dim) {
  switch (dim) {
    case WGPUTextureDimension_1D:
      return "1d";
    case WGPUTextureDimension_2D:
      return "2d";
    case WGPUTextureDimension_3D:
      return "3d";
    default:
      return "2d";
  }
}

WGPUTextureViewDimension StringToWGPUTextureViewDimension(const std::string &str) {
  if (str == "1d") {
    return WGPUTextureViewDimension_1D;
  }
  if (str == "2d") {
    return WGPUTextureViewDimension_2D;
  }
  if (str == "2d-array") {
    return WGPUTextureViewDimension_2DArray;
  }
  if (str == "cube") {
    return WGPUTextureViewDimension_Cube;
  }
  if (str == "cube-array") {
    return WGPUTextureViewDimension_CubeArray;
  }
  if (str == "3d") {
    return WGPUTextureViewDimension_3D;
  }
  return WGPUTextureViewDimension_Undefined;
}

WGPUTextureAspect StringToWGPUTextureAspect(const std::string &str) {
  if (str == "all") {
    return WGPUTextureAspect_All;
  }
  if (str == "depth-only") {
    return WGPUTextureAspect_DepthOnly;
  }
  if (str == "stencil-only") {
    return WGPUTextureAspect_StencilOnly;
  }
  return WGPUTextureAspect_All;
}

const char *WGPUFeatureNameToString(WGPUFeatureName name) {
  switch (name) {
    case WGPUFeatureName_DepthClipControl:
      return "depth-clip-control";
    case WGPUFeatureName_Depth32FloatStencil8:
      return "depth32float-stencil8";
    case WGPUFeatureName_TextureCompressionBC:
      return "texture-compression-bc";
    case WGPUFeatureName_TextureCompressionETC2:
      return "texture-compression-etc2";
    case WGPUFeatureName_TextureCompressionASTC:
      return "texture-compression-astc";
    case WGPUFeatureName_TimestampQuery:
      return "timestamp-query";
    case WGPUFeatureName_IndirectFirstInstance:
      return "indirect-first-instance";
    case WGPUFeatureName_ShaderF16:
      return "shader-f16";
    case WGPUFeatureName_RG11B10UfloatRenderable:
      return "rg11b10ufloat-renderable";
    case WGPUFeatureName_BGRA8UnormStorage:
      return "bgra8unorm-storage";
    case WGPUFeatureName_Float32Filterable:
      return "float32-filterable";
    case WGPUFeatureName_Force32:
    case WGPUFeatureName_Undefined:
      return NULL;
  }
  return NULL;
}

WGPUFeatureName StringToWGPUFeatureName(const std::string &name) {
  if (name == "depth-clip-control") {
    return WGPUFeatureName_DepthClipControl;
  }
  if (name == "depth32float-stencil8") {
    return WGPUFeatureName_Depth32FloatStencil8;
  }
  if (name == "texture-compression-bc") {
    return WGPUFeatureName_TextureCompressionBC;
  }
  if (name == "texture-compression-etc2") {
    return WGPUFeatureName_TextureCompressionETC2;
  }
  if (name == "texture-compression-astc") {
    return WGPUFeatureName_TextureCompressionASTC;
  }
  if (name == "timestamp-query") {
    return WGPUFeatureName_TimestampQuery;
  }
  if (name == "indirect-first-instance") {
    return WGPUFeatureName_IndirectFirstInstance;
  }
  if (name == "shader-f16") {
    return WGPUFeatureName_ShaderF16;
  }
  if (name == "rg11b10ufloat-renderable") {
    return WGPUFeatureName_RG11B10UfloatRenderable;
  }
  if (name == "bgra8unorm-storage") {
    return WGPUFeatureName_BGRA8UnormStorage;
  }
  if (name == "float32-filterable") {
    return WGPUFeatureName_Float32Filterable;
  }
  return WGPUFeatureName_Undefined;
}

WGPUBufferBindingType StringToWGPUBufferBindingType(const std::string &name) {
  if (name == "uniform") {
    return WGPUBufferBindingType_Uniform;
  }
  if (name == "storage") {
    return WGPUBufferBindingType_Storage;
  }
  if (name == "read-only-storage") {
    return WGPUBufferBindingType_ReadOnlyStorage;
  }
  return WGPUBufferBindingType_Undefined;
}

WGPUQueryType StringToWGPUQueryType(const std::string &name) {
  if (name == "occlusion") {
    return WGPUQueryType_Occlusion;
  }
  if (name == "timestamp") {
    return WGPUQueryType_Timestamp;
  }
  return WGPUQueryType_Occlusion;
}

const char *WGPUQuerySetToString(WGPUQueryType type) {
  switch (type) {
    case WGPUQueryType_Occlusion:
      return "occlusion";
    case WGPUQueryType_Timestamp:
      return "timestamp";
    default:
      return NULL;
  }
}

WGPUIndexFormat StringToWGPUIndexFormat(const std::string &str) {
  if (str == "uint16") {
    return WGPUIndexFormat_Uint16;
  }
  if (str == "uint32") {
    return WGPUIndexFormat_Uint32;
  }
  return WGPUIndexFormat_Undefined;
}

WGPUSamplerBindingType StringToWGPUSamplerBindingType(const std::string &str) {
  if (str == "filtering") {
    return WGPUSamplerBindingType_Filtering;
  }
  if (str == "non-filtering") {
    return WGPUSamplerBindingType_NonFiltering;
  }
  if (str == "comparison") {
    return WGPUSamplerBindingType_Comparison;
  }
  return WGPUSamplerBindingType_Undefined;
}

WGPUTextureSampleType StringToWGPUTextureSampleType(const std::string &str) {
  if (str == "float") {
    return WGPUTextureSampleType_Float;
  }
  if (str == "unfilterable-float") {
    return WGPUTextureSampleType_UnfilterableFloat;
  }
  if (str == "depth") {
    return WGPUTextureSampleType_Depth;
  }
  if (str == "sint") {
    return WGPUTextureSampleType_Sint;
  }
  if (str == "uint") {
    return WGPUTextureSampleType_Uint;
  }
  return WGPUTextureSampleType_Undefined;
}

WGPUAddressMode StringToWGPUAddressMode(const std::string &str) {
  if (str == "repeat") {
    return WGPUAddressMode_Repeat;
  }
  if (str == "mirror-repeat") {
    return WGPUAddressMode_MirrorRepeat;
  }
  if (str == "clamp-to-edge") {
    return WGPUAddressMode_ClampToEdge;
  }
  return WGPUAddressMode_Repeat;
}

const char *WGPUBufferMapStateToString(WGPUBufferMapState state) {
  switch (state) {
    case WGPUBufferMapState_Unmapped:
      return "unmapped";
    case WGPUBufferMapState_Pending:
      return "pending";
    case WGPUBufferMapState_Mapped:
      return "mapped";
    default:
      return NULL;
  }
}

WGPUStorageTextureAccess StringToWGPUStorageTextureAccess(const std::string &str) {
  if (str == "write-only") {
    return WGPUStorageTextureAccess_WriteOnly;
  }
  if (str == "read-only") {
    return WGPUStorageTextureAccess_ReadOnly;
  }
  if (str == "read-write") {
    return WGPUStorageTextureAccess_ReadWrite;
  }
  return WGPUStorageTextureAccess_Undefined;
}
