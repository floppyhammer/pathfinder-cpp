#include "vector_server.h"

namespace Pathfinder {
    void VectorServer::init(const std::shared_ptr<Pathfinder::Driver> &driver, float p_canvas_width, float p_canvas_height, const std::vector<char> &area_lut_input) {
        canvas = std::make_shared<Canvas>(driver, p_canvas_width, p_canvas_height, area_lut_input);
    }
}
