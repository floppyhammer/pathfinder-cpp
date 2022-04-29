#ifndef PATHFINDER_GPU_FRAMEBUFFER_GL_H
#define PATHFINDER_GPU_FRAMEBUFFER_GL_H

#include "texture.h"
#include "../framebuffer.h"

#include <memory>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class FramebufferGl : public Framebuffer {
    public:
        /// Normal framebuffer.
        FramebufferGl(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        /// Swap chain framebuffer.
        FramebufferGl(uint32_t p_width, uint32_t p_height);

        ~FramebufferGl();

        uint32_t get_gl_framebuffer() const;

        uint32_t get_unique_id() override;

    private:
        uint32_t gl_framebuffer;
    };
}

#endif

#endif //PATHFINDER_GPU_FRAMEBUFFER_GL_H
