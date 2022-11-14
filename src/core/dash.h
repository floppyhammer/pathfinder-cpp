#ifndef PATHFINDER_DASH_H
#define PATHFINDER_DASH_H

#include "data/path.h"

namespace Pathfinder {

struct DashState {
    Contour output;
    std::vector<float> dashes;
    size_t current_dash_index;
    float distance_left;

    DashState(const std::vector<float> &_dashes, float _offset);

    bool is_on() const;
};

/// Transforms a stroke into a dashed stroke.
struct OutlineDash {
    Outline &input;
    Outline output;
    DashState state;

    OutlineDash(Outline &_input, const std::vector<float> &dashes, float offset);

    void dash();

    Outline into_outline();
};

struct ContourDash {
    Contour &input;
    Outline &output;
    DashState &state;

    ContourDash(Contour &_input, Outline &_output, DashState &_state);

    void dash();
};

} // namespace Pathfinder

#endif // PATHFINDER_DASH_H
