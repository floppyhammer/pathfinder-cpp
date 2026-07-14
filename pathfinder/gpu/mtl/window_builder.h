#pragma once

#include "../window_builder.h"

namespace Pathfinder {

class WindowBuilderMtl final : public WindowBuilder {
public:
    explicit WindowBuilderMtl(const Vec2I &logical_size);

    ~WindowBuilderMtl() override;

    uint8_t create_window(const Vec2I &logical_size, const std::string &title) override;

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;

private:
    void *mtl_device_ = nullptr;
    void *mtl_cmd_queue_ = nullptr;
};

} // namespace Pathfinder
