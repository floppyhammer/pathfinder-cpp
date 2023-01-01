#include "transform2.h"

namespace Pathfinder {

Transform2::Transform2() {
    *this = Transform2::from_scale(Vec2F(1));
}

Transform2::Transform2(float xform[6]) {
    matrix = {xform[0], xform[1], xform[2], xform[3]};
    vector = {xform[4], xform[5]};
}

Transform2::Transform2(Mat2x2<float> _matrix, Vec2F _vector) : matrix(_matrix), vector(_vector) {}

Transform2 Transform2::translate(Vec2F _vector) const {
    return Transform2::from_translation(_vector) * *this;
}

Transform2 Transform2::rotate(float theta) const {
    return Transform2::from_rotation(theta) * *this;
}

bool Transform2::is_identity() const {
    return matrix == Mat2x2<float>::from_scale({1, 1}) && vector == Vec2F();
}

Transform2 Transform2::inverse() const {
    auto matrix_inv = matrix.inverse();
    auto vector_inv = -(matrix_inv * vector);
    return {matrix_inv, vector_inv};
}

Vec2F Transform2::get_position() const {
    return vector;
}

} // namespace Pathfinder
