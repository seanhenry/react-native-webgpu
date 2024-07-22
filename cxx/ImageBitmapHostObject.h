#pragma once

#include <jsi/jsi.h>

using namespace facebook::jsi;

namespace wgpu {

class ImageBitmapHostObject: public HostObject {
public:
    explicit ImageBitmapHostObject(uint8_t *data, size_t size, uint32_t width, uint32_t height): _data(data), _size(size), _width(width), _height(height) {}

    ~ImageBitmapHostObject() {
        destroy();
    }

    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;

    void destroy() {
        if (_data != nullptr) {
            _data = nullptr;
            free(_data);
        }
    }

    uint32_t _width;
    uint32_t _height;
    uint8_t *_data;
    size_t _size;
};

}
