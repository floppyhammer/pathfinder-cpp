#ifndef PATHFINDER_VECTOR_SERVER_H
#define PATHFINDER_VECTOR_SERVER_H

#include "../../d3dx/canvas.h"

namespace Pathfinder {
    /**
     * All visible shapes will be collected by the vector server and drawn at once.
     */
    class VectorServer {
    public:
        static VectorServer &get_singleton() {
            static VectorServer singleton;
            return singleton;
        }

        void init(float p_canvas_width, float p_canvas_height, const std::vector<char> &area_lut_input);

        std::shared_ptr<Canvas> canvas;
    };
}

#endif //PATHFINDER_VECTOR_SERVER_H
