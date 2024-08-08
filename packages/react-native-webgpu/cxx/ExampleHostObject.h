// #pragma once
//
// #include <jsi/jsi.h>
// #include "WGPUContext.h"
// #include "webgpu.h"
//
// using namespace facebook::jsi;
//
// namespace wgpu {
//
// class ExampleHostObject : public HostObject {
// public:
//     explicit ExampleHostObject(WGPUExample value, std::shared_ptr<WGPUContext> context,
//     std::string label): _value(value), _context(context), _label(label) {} ~ExampleHostObject() {
//     wgpuExampleRelease(_value); } std::vector<PropNameID> getPropertyNames(Runtime& runtime)
//     override; Value get(Runtime &runtime, const PropNameID &name) override; WGPUExample _value;
//     std::shared_ptr<WGPUContext> _context;
//     std::string _label;
// };
//
// }
