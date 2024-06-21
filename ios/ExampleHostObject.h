//#pragma once
//
//#include <jsi/jsi.h>
//#include "WGPUContext.h"
//#include "wgpu.h"
//
//using namespace facebook::jsi;
//
//namespace wgpu {
//
//class ExampleHostObject : public HostObject {
//public:
//    explicit ExampleHostObject(WGPUExample value, WGPUContext *context): _value(value), _context(context) {}
//    ~ExampleHostObject() { wgpuExampleRelease(_value); }
//    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
//    Value get(Runtime &runtime, const PropNameID &name) override;
//    WGPUExample _value;
//    WGPUContext *_context;
//};
//
//}
