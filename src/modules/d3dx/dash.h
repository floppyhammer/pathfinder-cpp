#ifndef PATHFINDER_DASH_H
#define PATHFINDER_DASH_H

#include "data/shape.h"

namespace Pathfinder {
    struct DashState {
        Path output;
        std::vector<float> dashes;
        size_t current_dash_index;
        float distance_left;

        DashState(const std::vector<float> &p_dashes, float p_offset);

        bool is_on() const;
    };

    /// Transforms a stroke into a dashed stroke.
    struct OutlineDash {
        Shape &input;
        Shape output;
        DashState state;

        OutlineDash(Shape &p_input, const std::vector<float> &p_dashes, float p_offset);

        void dash();

        Shape into_outline();
    };

    struct ContourDash {
        Path &input;
        Shape &output;
        DashState &state;

        ContourDash(Path &p_input, Shape &p_output, DashState &p_state);

        void dash();
    };
}

#endif //PATHFINDER_DASH_H
