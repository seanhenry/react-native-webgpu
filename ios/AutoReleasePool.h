#pragma once

namespace wgpu {

class AutoReleasePool {
public:
    AutoReleasePool() = default;

    template <typename T>
    void add(std::shared_ptr<T> ptr) {
        pool.push_back(ptr);
    }

    void clear() {
        pool.clear();
    }

    ~AutoReleasePool() {
        pool.clear();
    }

private:
    std::vector<std::shared_ptr<void>> pool;
};

}
