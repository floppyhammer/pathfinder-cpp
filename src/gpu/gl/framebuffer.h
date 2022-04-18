#ifndef PATHFINDER_GPU_FRAMEBUFFER_GL_H
#define PATHFINDER_GPU_FRAMEBUFFER_GL_H

#include "texture.h"
#include "../framebuffer.h"

#include <memory>

namespace Pathfinder {
    class FramebufferGl : public Framebuffer {
    public:
        /// To screen viewport.
        FramebufferGl(int p_width, int p_height);

        /// To texture.
        FramebufferGl(int p_width, int p_height, TextureFormat p_format, DataType p_type);

        ~FramebufferGl();

        uint32_t get_framebuffer_id() const;

        std::shared_ptr<Texture> get_texture() override;

        uint32_t get_unique_id() override;

    private:
        uint32_t framebuffer_id = 0;

        // Only valid when drawing to a texture.
        std::shared_ptr<TextureGl> texture;
    };
}

#endif //PATHFINDER_GPU_FRAMEBUFFER_GL_H
