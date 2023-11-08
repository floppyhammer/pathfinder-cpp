#ifndef PATHFINDER_GPU_COMMAND_BUFFER_GL_H
#define PATHFINDER_GPU_COMMAND_BUFFER_GL_H

#include <cstdint>
#include <memory>
#include <queue>

#include "../command_encoder.h"

namespace Pathfinder {

class CommandEncoderGl : public CommandEncoder {
    friend class DeviceGl;
    friend class SwapChainGl;

public:
    ~CommandEncoderGl() override;

private:
    CommandEncoderGl() = default;

    bool finish() override;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMMAND_BUFFER_GL_H
