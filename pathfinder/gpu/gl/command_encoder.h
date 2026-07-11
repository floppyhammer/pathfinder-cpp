#pragma once

#include <cstdint>

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

    std::vector<uint32_t> vao_;
};

} // namespace Pathfinder
