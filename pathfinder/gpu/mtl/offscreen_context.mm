#include "offscreen_context.h"

#include "device.h"
#include "queue.h"

namespace Pathfinder {

OffscreenContextMtl::OffscreenContextMtl(void* device, void* mtl_cmd_queue, uint32_t frames_in_flight)
    : OffscreenContext(frames_in_flight) {
    mtl_device_ = device;
    mtl_cmd_queue_ = mtl_cmd_queue;
}

OffscreenContextMtl::~OffscreenContextMtl() {}

std::shared_ptr<Device> OffscreenContextMtl::request_device() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device_;
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)mtl_cmd_queue_;
    return std::make_shared<DeviceMtl>(device, queue, frames_in_flight_);
}

std::shared_ptr<Queue> OffscreenContextMtl::create_queue() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device_;
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)mtl_cmd_queue_;

    return std::shared_ptr<QueueMtl>(new QueueMtl(device, queue, frames_in_flight_));
}

} // namespace Pathfinder
