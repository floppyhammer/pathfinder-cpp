#ifndef PATHFINDER_CONTROL_H
#define PATHFINDER_CONTROL_H

#include "node.h"
#include "../servers/vector_server.h"
#include "../resources/style_box.h"
#include "../../../common/math/vec2.h"
#include "../../d3dx/canvas.h"
#include "../../d3dx/scene.h"
#include "../../d3d9/scene_builder.h"
#include "../../d3d9/renderer.h"

namespace Pathfinder {
    class Control : public Node {
    public:
        Control();

        void set_rect_position(float x, float y);

        Vec2<float> get_rect_position() const;

        void set_rect_size(float w, float h);

        Vec2<float> get_rect_size() const;

        void set_rect_scale(float x, float y);

        Vec2<float> get_rect_scale() const;

        void set_rect_rotation(float r);

        float get_rect_rotation() const;

        void set_rect_pivot_offset(float x, float y);

        Vec2<float> get_rect_pivot_offset() const;

        void handle_input_events();

        void set_style_box(const StyleBox &p_style_box);

        StyleBox get_style_box() const;

        virtual Vec2<float> calculate_minimum_size() const;

        Vec2<float> get_minimum_size() const;

    protected:
        Vec2<float> rect_position = Vec2<float>(0);

        Vec2<float> rect_size = Vec2<float>(128);

        Vec2<float> rect_scale = Vec2<float>(1);

        float rect_rotation = 0;

        Vec2<float> rect_pivot_offset = Vec2<float>(0);

        Vec2<float> minimum_size = Vec2<float>(0);

        StyleBox style_box;

    public:
        virtual void update();

        virtual void draw();
    };
}

#endif //PATHFINDER_CONTROL_H
