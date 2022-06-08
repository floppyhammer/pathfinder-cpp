#ifndef PATHFINDER_DASH_H
#define PATHFINDER_DASH_H

#include "data/shape.h"

namespace Pathfinder {
    struct DashState {
        Contour output;
        std::vector<float> dashes;
        size_t current_dash_index;
        float distance_left;

        DashState(const std::vector<float> &p_dashes, float p_offset);

        bool is_on() const;
    };

    /// Transforms a stroke into a dashed stroke.
    struct OutlineDash {
        Outline &input;
        Outline output;
        DashState state;

        OutlineDash(Outline &p_input, const std::vector<float> &p_dashes, float p_offset);

        void dash();

        Outline into_outline();
    };

    struct ContourDash {
        Contour &input;
        Outline &output;
        DashState &state;

        ContourDash(Contour &p_input, Outline &p_output, DashState &p_state);

        void dash();
    };
}

#endif //PATHFINDER_DASH_H
