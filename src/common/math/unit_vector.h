#ifndef PATHFINDER_UNIT_VECTOR_H
#define PATHFINDER_UNIT_VECTOR_H

#include "vec2.h"

#include <cmath>

namespace Pathfinder {
    struct UnitVector {
        float x;
        float y;

        UnitVector(float p_x, float p_y);

        explicit UnitVector(const Vec2<float> &p_value);

        static UnitVector from_angle(float theta) {
            return {std::cos(theta), std::sin(theta)};
        }

        /// Angle addition formula.
        UnitVector rotate_by(const UnitVector &other) const;

        /// Angle subtraction formula.
        UnitVector rev_rotate_by(const UnitVector &other) const;

        /// Half angle formula.
        UnitVector halve_angle() const;
    };
}

#endif //PATHFINDER_UNIT_VECTOR_H
