#ifndef PATHFINDER_WINDOW_BUILDER_VK_H
#define PATHFINDER_WINDOW_BUILDER_VK_H

#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"
#include "../window_builder.h"
#include "device.h"

namespace Pathfinder {

/// List of required validation layers.
const std::vector<const char *> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

/// List of required device extensions.
/// VK_EXT_DEBUG_MARKER_EXTENSION_NAME shouldn't go here as it's optional and is only used for debug reason.
const std::vector<const char *> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector<const char *> INSTANCE_EXTENSIONS = {
    "VK_KHR_surface",
    #ifdef __ANDROID__
    "VK_KHR_android_surface",
    #endif
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

VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);

VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes);

SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice _physical_device, VkSurfaceKHR surface);

QueueFamilyIndices find_queue_families(VkPhysicalDevice _physical_device, VkSurfaceKHR surface);

class Window;

class WindowBuilderVk : public WindowBuilder {
public:
    #ifndef __ANDROID__
    explicit WindowBuilderVk(const Vec2I &size);
    #else
    explicit WindowBuilderVk(ANativeWindow *native_window, const Vec2I &window_size);
    #endif

    ~WindowBuilderVk() override;

    // Call this right after the render loop is stopped.
    void preapre_destruction() override;

    std::shared_ptr<Window> create_window(const Vec2I &_size, const std::string &title) override;

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;

    VkPhysicalDevice get_physical_device() const;

    VkDevice get_device() const;

private:
    bool initialized = false;

    /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
    VkPhysicalDevice physical_device{};

    /// Logical device.
    VkDevice vk_device{};

    VkInstance instance{};
    VkDebugUtilsMessengerEXT debug_messenger{};

    static const bool enable_validation_layers =
    #if defined(PATHFINDER_DEBUG) && !defined(__ANDROID__)
        true;
    #else
        false;
    #endif

    VkQueue graphics_queue{};
    VkQueue present_queue{};

    VkCommandPool command_pool{};

    #ifdef __ANDROID__
    ANativeWindow *native_window_;
    #endif

private:
    #ifndef __ANDROID__
    static GLFWwindow *glfw_window_init(const Vec2I &size,
                                        const std::string &title,
                                        GLFWwindow *shared_window = nullptr);
    #endif

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                         const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                         void *user_data) {
        std::cerr << "Validation layer: " << callback_data->pMessage << std::endl;
        return VK_FALSE;
    }

    static bool check_validation_layer_support();

    [[nodiscard]] VkFormat find_depth_format() const;

    [[nodiscard]] VkFormat find_supported_format(const std::vector<VkFormat> &candidates,
                                                 VkImageTiling tiling,
                                                 VkFormatFeatureFlags features) const;

    static std::vector<const char *> get_required_instance_extensions();

    void initialize_after_surface_creation(VkSurfaceKHR surface);

    void setup_debug_messenger();

    void create_queues(VkSurfaceKHR surface);

    void create_instance();

    bool check_device_extension_support(VkPhysicalDevice physical_device) const;

    /**
     * Check if a physical device is suitable.
     * @param pPhysicalDevice
     * @return
     */
    bool is_device_suitable(VkPhysicalDevice _physical_device, VkSurfaceKHR surface) const;

    void pick_physical_device(VkSurfaceKHR surface);

    void create_logical_device(VkSurfaceKHR surface);

    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);

    void create_command_pool(VkSurfaceKHR surface);
};

} // namespace Pathfinder

#endif // PATHFINDER_WINDOW_BUILDER_VK_H
