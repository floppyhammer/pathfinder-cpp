//
// Created by chy on 2021/12/31.
//

#include "device_gl.h"

#include "../common/logger.h"

#include <iostream>
#include <sstream>

namespace Pathfinder {
    void DeviceGl::check_error(const char *flag) {
#ifdef PATHFINDER_DEBUG
        for (GLint error = glGetError(); error; error = glGetError()) {
            std::ostringstream string_stream;
            string_stream << "Error " << error << " after " << flag;
            Logger::error(string_stream.str(), "OpenGL");
        }
#endif
    }

    void DeviceGl::print_string(const char *name, GLenum s) {
#ifdef PATHFINDER_DEBUG
        const char *v = (const char *) glGetString(s);

        std::ostringstream string_stream;
        string_stream << "GL " << name << " = " << v;

        Logger::error(string_stream.str(), "OpenGL");
#endif
    }
}
