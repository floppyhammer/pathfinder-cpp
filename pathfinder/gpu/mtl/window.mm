#include "window.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#import <Cocoa/Cocoa.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#import <QuartzCore/CAMetalLayer.h>

#include "device.h"
#include "swap_chain.h"

namespace Pathfinder {

WindowMtl::WindowMtl(const Vec2I& size, void* glfw_window) : Window(size, glfw_window) {
    NSWindow* nswin = glfwGetCocoaWindow((GLFWwindow*)glfw_window);
    NSView* view = nswin.contentView;

    [view setWantsLayer:YES];
    metal_layer_ = [CAMetalLayer layer];
    [view setLayer:metal_layer_];

    // Set layer properties
    metal_layer_.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metal_layer_.contentsScale = nswin.backingScaleFactor;
}

WindowMtl::~WindowMtl() {
    destroy();
}

void WindowMtl::destroy() {
    if (glfw_window_) {
        glfwDestroyWindow((GLFWwindow*)glfw_window_);
        glfw_window_ = nullptr;
    }
}

std::shared_ptr<SwapChain> WindowMtl::get_swap_chain(const std::shared_ptr<Device>& device,
                                                  PresentMode present_mode) {
    if (!swapchain_) {
        swapchain_ = std::make_shared<SwapChainMtl>(
            physical_size_, std::static_pointer_cast<DeviceMtl>(device), metal_layer_, present_mode);
    }
    return swapchain_;
}

} // namespace Pathfinder
