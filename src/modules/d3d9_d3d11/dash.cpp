//
// Created by chy on 2021/10/9.
//

#include "dash.h"

#include "cmath"

namespace Pathfinder {
    const float EPSILON = 0.0001;

    DashState::DashState(const std::vector<float> &p_dashes, float p_offset)
            : dashes(p_dashes) {
        float total = 0;
        for (auto &n: p_dashes)
            total += n;
        p_offset = fmod(p_offset, total);

        current_dash_index = 0;
        while (current_dash_index < dashes.size()) {
            auto dash = dashes[current_dash_index];
            if (p_offset < dash)
                break;

            p_offset -= dash;
            current_dash_index += 1;
        }

        output = Path();
        distance_left = p_offset;
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
    OutlineDash::OutlineDash(Shape &p_input, const std::vector<float> &p_dashes, float p_offset)
            : input(p_input), output(Shape()), state(DashState(p_dashes, p_offset)) {}

    void OutlineDash::dash() {
        for (auto &contour: input.paths) {
            ContourDash(contour, output, state).dash();
        }
    }

    Shape OutlineDash::into_outline() {
        if (state.is_on()) {
            output.push_path(state.output);
        }

        return output;
    }

    ContourDash::ContourDash(Path &p_input, Shape &p_output, DashState &p_state)
            : input(p_input), output(p_output), state(p_state) {}

    void ContourDash::dash() {
        auto segments_iter = SegmentsIter(input.points, input.flags, input.closed);

        Segment queued_segment;
        bool queued_segment_is_none = true;

        // Traverse curve/line segments.
        while (true) {
            if (queued_segment_is_none) {
                if (!segments_iter.is_at_end()) {
                    queued_segment = segments_iter.get_next();
                    if (queued_segment.kind == SegmentKind::None)
                        break;
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
                    output.push_path(state.output);
                    state.output = Path();
                }

                state.current_dash_index += 1;
                if (state.current_dash_index == state.dashes.size()) {
                    state.current_dash_index = 0;
                }

                state.distance_left = state.dashes[state.current_dash_index];
            }
        }
    }
}
