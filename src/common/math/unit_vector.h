#ifndef PATHFINDER_UNIT_VECTOR_H
#define PATHFINDER_UNIT_VECTOR_H

#include <cmath>

#include "vec2.h"

namespace Pathfinder {

struct UnitVector {
    float x;
    float y;

    UnitVector(float _x, float _y);

    explicit UnitVector(const Vec2F &_vec2);

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

} // namespace Pathfinder

#endif // PATHFINDER_UNIT_VECTOR_H
