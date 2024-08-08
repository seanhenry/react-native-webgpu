#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class RenderPipelineHostObject : public HostObject {
public:
    explicit RenderPipelineHostObject(WGPURenderPipeline value, std::shared_ptr<WGPUContext> context, std::string label): _value(value), _context(context), _label(label) {}
    ~RenderPipelineHostObject() { wgpuRenderPipelineRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    inline WGPURenderPipeline getValue() { return _value; }
private:
    WGPURenderPipeline _value;
    std::shared_ptr<WGPUContext> _context;
    std::string _label;
};

}
