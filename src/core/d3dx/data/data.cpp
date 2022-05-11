#include "data.h"

namespace Pathfinder {
    const uint8_t PushSegmentFlags::UPDATE_BOUNDS = 0x01;
    const uint8_t PushSegmentFlags::INCLUDE_FROM_POINT = 0x02;

    const uint8_t PointFlags::CONTROL_POINT_0 = 0x01;
    const uint8_t PointFlags::CONTROL_POINT_1 = 0x02;
}
