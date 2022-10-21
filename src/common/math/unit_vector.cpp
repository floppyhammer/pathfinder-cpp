#include "unit_vector.h"

namespace Pathfinder {

UnitVector::UnitVector(float p_x, float p_y) : x(p_x), y(p_y) {}

UnitVector::UnitVector(const Vec2F &p_v) : x(p_v.x), y(p_v.y) {}

UnitVector UnitVector::rotate_by(const UnitVector &other) const {
    return {x * other.x - y * other.y, y * other.x + x * other.y};
}

UnitVector UnitVector::rev_rotate_by(const UnitVector &other) const {
    return {x * other.x + y * other.y, y * other.x - x * other.y};
}

UnitVector UnitVector::halve_angle() const {
    auto term = Vec2F(x, -x);
    return UnitVector((Vec2F(0.5f) * (Vec2F(1.0f) + term)).max(Vec2F(0)).sqrt());
}

} // namespace Pathfinder
