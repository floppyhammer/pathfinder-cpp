//
// Created by floppyhammer on 7/9/2021.
//

#include "unit_vector.h"

namespace Pathfinder {
    UnitVector::UnitVector(float p_x, float p_y) : x(p_x), y(p_y) {}

    UnitVector::UnitVector(const Vec2<float> &p_v) : x(p_v.x), y(p_v.y) {}

    UnitVector UnitVector::rotate_by(const UnitVector &other) const {
        return {x * other.x - y * other.y, y * other.x + x * other.y};
    }

    UnitVector UnitVector::rev_rotate_by(const UnitVector &other) const {
        return {x * other.x + y * other.y, y * other.x - x * other.y};
    }

    UnitVector UnitVector::halve_angle() const {
        auto term = Vec2<float>(x, -x);
        return UnitVector((Vec2<float>(0.5f) * (Vec2<float>(1.0f) + term)).max(Vec2<float>(0)).sqrt());
    }
}
