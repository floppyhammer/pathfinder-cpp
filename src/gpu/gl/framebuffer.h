#ifndef PATHFINDER_GPU_FRAMEBUFFER_GL_H
#define PATHFINDER_GPU_FRAMEBUFFER_GL_H

#include "texture.h"
#include "../framebuffer.h"

#include <memory>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class FramebufferGl : public Framebuffer {
    public:
        /// To screen viewport.
        FramebufferGl(uint32_t p_width, uint32_t p_height);

        /// To texture.
        FramebufferGl(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        ~FramebufferGl();

        uint32_t get_framebuffer_id() const;

        std::shared_ptr<Texture> get_texture() override;

        uint32_t get_unique_id() override;

    private:
        uint32_t framebuffer_id;

        // Only valid when drawing to a texture.
        std::shared_ptr<TextureGl> texture;
    };
}

#endif

#endif //PATHFINDER_GPU_FRAMEBUFFER_GL_H
