//#include "ExampleHostObject.h"
//#include "WGPUJsiUtils.h"
//#include "WGPUContext.h"
//
//using namespace facebook::jsi;
//using namespace wgpu;
//
//Value ExampleHostObject::get(Runtime &runtime, const PropNameID &propName) {
//    auto name = propName.utf8(runtime);
//
//    WGPU_LOG_GET_PROP;
//
//    if (name == "label") {
//        return String::createFromUtf8(runtime, _label);
//    }
//
//    WGPU_LOG_UNIMPLEMENTED_GET_PROP;
//
//    return Value::undefined();
//}
//
//std::vector<PropNameID> ExampleHostObject::getPropertyNames(Runtime& runtime) {
//    return PropNameID::names(runtime, "label");
//}
