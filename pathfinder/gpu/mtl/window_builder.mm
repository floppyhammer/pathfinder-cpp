#include "window_builder.h"

#include "device.h"
#include "queue.h"
#include "window.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Pathfinder {

WindowBuilderMtl::WindowBuilderMtl(const Vec2I& logical_size) {
    glfwInit();

    // To not create an OpenGL context (as we're using Metal).
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    float dpi_scaling_factor;
    auto glfw_window = glfw_window_init(logical_size, PRIMARY_WINDOW_TITLE, dpi_scaling_factor, false, nullptr);

    auto physical_size = (logical_size.to_f32() * dpi_scaling_factor).to_i32();

    primary_window_ = std::make_shared<WindowMtl>(physical_size, glfw_window);
    primary_window_->set_dpi_scaling_factor(dpi_scaling_factor);

    auto mtl_device = MTLCreateSystemDefaultDevice();
    auto mtl_cmd_queue = [mtl_device newCommandQueue];

    mtl_device_ = (void*)CFBridgingRetain(mtl_device);
    mtl_cmd_queue_ = (void*)CFBridgingRetain(mtl_cmd_queue);
}

WindowBuilderMtl::~WindowBuilderMtl() {
    if (mtl_device_) CFRelease((CFTypeRef)mtl_device_);
    if (mtl_cmd_queue_) CFRelease((CFTypeRef)mtl_cmd_queue_);

    glfwTerminate();
}

uint8_t WindowBuilderMtl::create_window(const Vec2I& logical_size, const std::string& title) {
    float dpi_scaling_factor;
    auto glfw_window = glfw_window_init(logical_size, title, dpi_scaling_factor, false, nullptr);

    auto physical_size = (logical_size.to_f32() * dpi_scaling_factor).to_i32();

    auto new_window = std::make_shared<WindowMtl>(physical_size, glfw_window);
    new_window->set_dpi_scaling_factor(dpi_scaling_factor);

    sub_windows_.push_back(new_window);
    new_window->window_index = sub_windows_.size();

    return sub_windows_.size();
}

std::shared_ptr<Device> WindowBuilderMtl::request_device() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device_;
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)mtl_cmd_queue_;
    return std::make_shared<DeviceMtl>(device, queue, MAX_FRAMES_IN_FLIGHT);
}

std::shared_ptr<Queue> WindowBuilderMtl::create_queue() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device_;
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)mtl_cmd_queue_;

    return std::shared_ptr<Queue>(new QueueMtl(device, queue, MAX_FRAMES_IN_FLIGHT));
}

} // namespace Pathfinder
