#ifndef PATHFINDER_GPU_QUEUE_GL_H
#define PATHFINDER_GPU_QUEUE_GL_H

#include <vector>

#include "../queue.h"
#include "fence.h"

namespace Pathfinder {

class QueueGl : public Queue {
    friend class WindowGl;

public:
    void submit(const std::shared_ptr<CommandEncoder> &encoder, const std::shared_ptr<Fence> &fence) override {
        if (encoder->submitted_) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        encoder->submitted_ = true;

        encoder->finish();

        auto fence_gl = (FenceGl *)fence.get();
        fence_gl->wait();
    }

    void submit_and_wait(const std::shared_ptr<CommandEncoder> &encoder) override {
        if (encoder->submitted_) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        encoder->submitted_ = true;

        encoder->finish();
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_QUEUE_GL_H
