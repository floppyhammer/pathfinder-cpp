#ifndef PATHFINDER_GPU_WINDOW_VK_H
#define PATHFINDER_GPU_WINDOW_VK_H

#include <cstring>
#include <iostream>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"
#include "device.h"

#ifdef PATHFINDER_USE_VULKAN
    // We don't need a Window on Android and Web.
    #if (defined(WIN32) || defined(__linux__) || defined(__APPLE__)) && !defined(ANDROID)

namespace Pathfinder {

/// List of required validation layers.
const std::vector<const char *> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

/// List of required device extensions.
/// VK_EXT_DEBUG_MARKER_EXTENSION_NAME shouldn't go here as it's optional and is only used for debug reason.
const std::vector<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

struct QueueFamilyIndices {
    std::shared_ptr<uint32_t> graphics_family;
    std::shared_ptr<uint32_t> present_family;

    [[nodiscard]] bool is_complete() const {
        return graphics_family && present_family;
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class WindowVk : public Window {
public:
    explicit WindowVk(Vec2I _size);

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;

    std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) override;

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);

    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes);

    static std::vector<const char *> get_required_extensions();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                         const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                         void *user_data) {
        std::cerr << "Validation layer: " << callback_data->pMessage << std::endl;

        return VK_FALSE;
    }

    static bool check_validation_layer_support();

    QueueFamilyIndices find_queue_families(VkPhysicalDevice _physical_device) const;

    SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice _physical_device) const;

    [[nodiscard]] VkFormat find_depth_format() const;

    [[nodiscard]] VkFormat find_supported_format(const std::vector<VkFormat> &candidates,
                                                 VkImageTiling tiling,
                                                 VkFormatFeatureFlags features) const;

    VkQueue get_present_queue() const {
        return present_queue;
    };

    void cleanup() override;

public:
    VkSurfaceKHR surface{};

    /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
    VkPhysicalDevice physical_device{};

    /// Logical device.
    VkDevice vk_device{};

private:
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debug_messenger{};

        #ifdef PATHFINDER_DEBUG
    static const bool enable_validation_layers = true;
        #else
    static const bool enable_validation_layers = false;
        #endif

    VkQueue graphics_queue{};
    VkQueue present_queue{};

    VkCommandPool command_pool{};

private:
    void init_glfw_window();

    void setup_debug_messenger();

    void create_instance();

    void create_surface();

    bool check_device_extension_support(VkPhysicalDevice physical_device) const;

    /**
     * Check if a physical device is suitable.
     * @param pPhysicalDevice
     * @return
     */
    bool is_device_suitable(VkPhysicalDevice _physical_device) const;

    void pick_physical_device();

    void create_logical_device();

    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);

    void create_command_pool();
};

} // namespace Pathfinder

    #endif
#endif

#endif // PATHFINDER_GPU_WINDOW_VK_H
