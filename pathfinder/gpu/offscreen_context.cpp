#include "offscreen_context.h"

namespace Pathfinder {

OffscreenContext::OffscreenContext(uint32_t frames_in_flight) {
    frames_in_flight_ = frames_in_flight;
}

} // namespace Pathfinder
