//
// Created by chy on 7/8/2021.
//

#include "transform2.h"

namespace Pathfinder {
    Transform2::Transform2() {
        *this = Transform2::from_scale(Vec2<float>(1));
    }

    Transform2::Transform2(Mat2x2<float> p_matrix, Vec2<float> p_vector) : matrix(p_matrix), vector(p_vector) {}

    Transform2 Transform2::translate(Vec2<float> p_vector) const {
        return Transform2::from_translation(p_vector) * *this;
    }

    Transform2 Transform2::inverse() const {
        auto matrix_inv = matrix.inverse();
        auto vector_inv = -(matrix_inv * vector);
        return {matrix_inv, vector_inv};
    }
}
