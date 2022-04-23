#include "framebuffer.h"

#include <cassert>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    FramebufferVk::FramebufferVk(int p_width, int p_height) : Framebuffer(p_width, p_height) {
        // Do nothing.
    }

    FramebufferVk::FramebufferVk(int p_width, int p_height, TextureFormat p_format, DataType p_type)
            : Framebuffer(p_width, p_height) {
        // Create a color texture.
        //texture = std::make_shared<TextureVk>(p_width, p_height, p_format, p_type);

    }

    FramebufferVk::~FramebufferVk() {
    }

    VkFramebuffer FramebufferVk::get_framebuffer_id() const {
        return framebuffer_id;
    }

    std::shared_ptr<Texture> FramebufferVk::get_texture() {
        assert(texture != nullptr && "Tried to get texture from screen viewport!");
        return texture;
    }

    uint32_t FramebufferVk::get_unique_id() {
        return reinterpret_cast<uint32_t>(&framebuffer_id);;
    }
}

#endif
