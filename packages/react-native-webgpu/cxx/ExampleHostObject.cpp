// #include "ExampleHostObject.h"
// #include "Mixins.h"
// #include "WGPUJsiUtils.h"
// #include "WGPUContext.h"
//
// using namespace facebook::jsi;
// using namespace wgpu;
//
// Value ExampleHostObject::get(Runtime &runtime, const PropNameID &propName) {
//     auto name = propName.utf8(runtime);
//
//     WGPU_LOG_GET_PROP;
//
//     WGPU_GET_LABEL()
//     WGPU_GET_BRAND(GPUExample)
//
//     WGPU_LOG_UNIMPLEMENTED_GET_PROP;
//
//     return Value::undefined();
// }
//
// std::vector<PropNameID> ExampleHostObject::getPropertyNames(Runtime& runtime) {
//     return PropNameID::names(runtime, "label", "__brand");
// }
