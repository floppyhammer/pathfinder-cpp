#ifndef PATHFINDER_D3D9_TILER_H
#define PATHFINDER_D3D9_TILER_H

#include "object_builder.h"
#include "../d3dx/data/data.h"

namespace Pathfinder {
    struct Outcode {
        uint8_t flag = 0x00;

        static const uint8_t LEFT = 0x01;
        static const uint8_t RIGHT = 0x02;
        static const uint8_t TOP = 0x04;
        static const uint8_t BOTTOM = 0x08;

        bool is_empty() const;

        bool contains(uint8_t p_side) const;

        inline Outcode operator&(const Outcode &b) const {
            Outcode res;
            res.flag = flag & b.flag;
            return res;
        }
    };

    /// This is the meat of the technique. It implements the fast lattice-clipping algorithm from
    /// Nehab and Hoppe, "Random-Access Rendering of General Vector Graphics" 2006.
    /// The algorithm to step through tiles is Amanatides and Woo, "A Fast Voxel Traversal Algorithm for
    /// Ray Tracing" 1987: http://www.cse.yorku.ca/~amana/research/grid.pdf
    void process_line_segment(LineSegmentF p_line_segment, SceneBuilderD3D9& p_scene_builder, ObjectBuilder& p_object_builder);

    /// Recursive call.
    void process_segment(Segment& p_segment, SceneBuilderD3D9& p_scene_builder, ObjectBuilder& p_object_builder);

    /// One tiler for one outline (shape).
    struct Tiler {
    public:
        Tiler(SceneBuilderD3D9 &p_scene_builder, uint32_t path_id, const Shape &p_shape,
              FillRule fill_rule, const Rect<float> &view_box);

        ObjectBuilder object_builder;

        /// Generate fills and tiles.
        void generate_tiles();

    private:
        SceneBuilderD3D9& scene_builder;
        Shape shape;

        /// Process all paths of the attached shape.
        void generate_fills();

        /// Prepare the winding (backdrops) vector for solid tiles.
        void prepare_tiles();
    };
}

#endif //PATHFINDER_D3D9_TILER_H
