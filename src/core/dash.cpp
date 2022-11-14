#include "dash.h"

#include <cmath>

namespace Pathfinder {

const float EPSILON = 0.0001;

DashState::DashState(const std::vector<float> &_dashes, float _offset) : dashes(_dashes) {
    float total = 0;
    for (auto &n : _dashes) {
        total += n;
    }
    _offset = fmod(_offset, total);

    current_dash_index = 0;
    while (current_dash_index < dashes.size()) {
        auto dash = dashes[current_dash_index];
        if (_offset < dash) break;

        _offset -= dash;
        current_dash_index += 1;
    }

    output = Contour();
    distance_left = _offset;
}

bool DashState::is_on() const {
    return current_dash_index % 2 == 0;
}

/// Creates a new outline dasher for the given stroke.
///
/// Arguments:
///
/// * `input`: The input stroke to be dashed. This must not yet been converted to a fill; i.e.
///   it is assumed that the stroke-to-fill conversion happens *after* this dashing process.
///
/// * `dashes`: The list of dashes, specified as alternating pixel lengths of lines and gaps
///   that describe the pattern. See
///   https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/setLineDash.
///
/// * `offset`: The line dash offset, or "phase". See
///   https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/lineDashOffset.
OutlineDash::OutlineDash(Outline &_input, const std::vector<float> &dashes, float offset)
    : input(_input), output(Outline()), state(DashState(dashes, offset)) {}

void OutlineDash::dash() {
    for (auto &contour : input.contours) {
        ContourDash(contour, output, state).dash();
    }
}

Outline OutlineDash::into_outline() {
    if (state.is_on()) {
        output.push_contour(state.output);
    }

    return output;
}

ContourDash::ContourDash(Contour &_input, Outline &_output, DashState &_state)
    : input(_input), output(_output), state(_state) {}

void ContourDash::dash() {
    auto segments_iter = SegmentsIter(input.points, input.flags, input.closed);

    Segment queued_segment;
    bool queued_segment_is_none = true;

    // Traverse curve/line segments.
    while (true) {
        if (queued_segment_is_none) {
            if (!segments_iter.is_at_end()) {
                queued_segment = segments_iter.get_next();
                if (queued_segment.kind == SegmentKind::None) break;
                queued_segment_is_none = false;
            } else {
                break;
            }
        }

        auto current_segment = queued_segment;
        auto distance = state.distance_left;

        auto t = distance / current_segment.arc_length();

        if (t < 1.0) {
            Segment prev_segment, next_segment;
            current_segment.split(t, prev_segment, next_segment);
            current_segment = prev_segment;
            queued_segment = next_segment;
            queued_segment_is_none = false;
        } else {
            distance = current_segment.arc_length();
            queued_segment_is_none = true;
        }

        if (state.is_on()) {
            state.output.push_segment(current_segment, PushSegmentFlags());
        }

        state.distance_left -= distance;

        if (state.distance_left < EPSILON) {
            if (state.is_on()) {
                output.push_contour(state.output);
                state.output = Contour();
            }

            state.current_dash_index += 1;
            if (state.current_dash_index == state.dashes.size()) {
                state.current_dash_index = 0;
            }

            state.distance_left = state.dashes[state.current_dash_index];
        }
    }
}

} // namespace Pathfinder
