#include "vector_server.h"

namespace Pathfinder {
    void VectorServer::init(float p_canvas_width, float p_canvas_height, const std::vector<unsigned char> &area_lut_input) {
        canvas = std::make_shared<Canvas>(p_canvas_width, p_canvas_height, area_lut_input);
    }
}
