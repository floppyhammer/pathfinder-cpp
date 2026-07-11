#pragma once

namespace Pathfinder {

class ShaderModuleVk : public ShaderModule {
    friend class DeviceVk;

public:
    ~ShaderModuleVk() override {
        vkDestroyShaderModule(vk_device_, vk_shader_module_, nullptr);
    }

    VkShaderModule get_raw_handle() const {
        return vk_shader_module_;
    }

private:
    ShaderModuleVk() = default;

    VkShaderModule vk_shader_module_{};
    VkDevice vk_device_{};
};

} // namespace Pathfinder
