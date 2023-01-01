#include "unit_vector.h"

namespace Pathfinder {

UnitVector::UnitVector(float _x, float _y) : x(_x), y(_y) {}

UnitVector::UnitVector(const Vec2F &_vec2) : x(_vec2.x), y(_vec2.y) {}

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
