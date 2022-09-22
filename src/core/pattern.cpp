#include "pattern.h"

namespace Pathfinder {
bool Pattern::repeat_x() const {
    return (flags.value & PatternFlags::REPEAT_X) != 0x0;
}

void Pattern::set_repeat_x(bool repeat_x) {
    if (repeat_x) {
        flags.value |= PatternFlags::REPEAT_X;
    } else {
        flags.value &= !PatternFlags::REPEAT_X;
    }
}

bool Pattern::repeat_y() const {
    return (flags.value & PatternFlags::REPEAT_Y) != 0x0;
}

void Pattern::set_repeat_y(bool repeat_y) {
    if (repeat_y) {
        flags.value |= PatternFlags::REPEAT_Y;
    } else {
        flags.value &= !PatternFlags::REPEAT_Y;
    }
}

bool Pattern::smoothing_enabled() const {
    return (flags.value & PatternFlags::NO_SMOOTHING) == 0x0;
}

void Pattern::set_smoothing_enabled(bool enable) {
    if (!enable) {
        flags.value |= PatternFlags::NO_SMOOTHING;
    } else {
        flags.value &= !PatternFlags::NO_SMOOTHING;
    }
}
} // namespace Pathfinder
