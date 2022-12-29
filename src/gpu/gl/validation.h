#ifndef PATHFINDER_GPU_VALIDATION_H
#define PATHFINDER_GPU_VALIDATION_H

#include <iostream>
#include <sstream>

#include "../../common/global_macros.h"
#include "../../common/logger.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

struct DebugMarker {
    inline static void label_buffer(GLuint object, const std::string &label) {
    #ifndef __ANDROID__
        if (GLAD_GL_EXT_debug_label) {
            glLabelObjectEXT(GL_BUFFER_OBJECT_EXT, object, 0, label.c_str());
        }
    #endif
    }

    inline static void label_shader(GLuint object, const std::string &label) {
    #ifndef __ANDROID__
        if (GLAD_GL_EXT_debug_label) {
            glLabelObjectEXT(GL_SHADER_OBJECT_EXT, object, 0, label.c_str());
        }
    #endif
    }

    inline static void label_program(GLuint object, const std::string &label) {
    #ifndef __ANDROID__
        if (GLAD_GL_EXT_debug_label) {
            glLabelObjectEXT(GL_PROGRAM_OBJECT_EXT, object, 0, label.c_str());
        }
    #endif
    }

    inline static void label_vao(GLuint object, const std::string &label) {
    #ifndef __ANDROID__
        if (GLAD_GL_EXT_debug_label) {
            glLabelObjectEXT(GL_VERTEX_ARRAY_OBJECT_EXT, object, 0, label.c_str());
        }
    #endif
    }

    inline static void label_texture(GLuint object, const std::string &label) {
    #ifndef __ANDROID__
        if (GLAD_GL_EXT_debug_label) {
            glLabelObjectEXT(GL_TEXTURE, object, 0, label.c_str());
        }
    #endif
    }

    inline static void label_framebuffer(GLuint object, const std::string &label) {
    #ifndef __ANDROID__
        if (GLAD_GL_EXT_debug_label) {
            glLabelObjectEXT(GL_FRAMEBUFFER, object, 0, label.c_str());
        }
    #endif
    }
};

inline void gl_check_error(const char *flag) {
    #ifdef PATHFINDER_DEBUG
    for (GLint error = glGetError(); error; error = glGetError()) {
        std::ostringstream string_stream;
        string_stream << "Error " << error << " after " << flag;
        Logger::error(string_stream.str(), "OpenGL");
    }
    #endif
}

inline void gl_print_string(const char *name, GLenum s) {
    #ifdef PATHFINDER_DEBUG
    const char *v = (const char *)glGetString(s);

    std::ostringstream string_stream;
    string_stream << "GL " << name << " = " << v;

    Logger::error(string_stream.str(), "OpenGL");
    #endif
}

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_VALIDATION_H
