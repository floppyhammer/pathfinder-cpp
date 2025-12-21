#ifndef PATHFINDER_WINDOW_BUILDER_VK_H
#define PATHFINDER_WINDOW_BUILDER_VK_H

#include <memory>
#include <vector>

#include "../../common/global_macros.h"
#include "../window.h"
#include "../window_builder.h"
#include "device.h"

namespace Pathfinder {

/// List of required validation layers.
const std::vector VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

/// List of required device extensions.
const std::vector DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector INSTANCE_EXTENSIONS = {
    "VK_KHR_surface",
#ifdef __ANDROID__
    "VK_KHR_android_surface",
#endif
};

struct QueueFamilyIndices {
    std::shared_ptr<uint32_t> graphics_family;
    std::shared_ptr<uint32_t> present_family;

    bool is_complete() const {
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
    explicit WindowBuilderVk(const Vec2I &logical_size);
#else
    explicit WindowBuilderVk(ANativeWindow *native_window, const Vec2I &physical_size);
#endif

    ~WindowBuilderVk() override;

    void stop_and_destroy_swapchains() override;

    uint8_t create_window(const Vec2I &size, const std::string &title) override;

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;

    VkPhysicalDevice get_physical_device() const;

    VkDevice get_device() const;

private:
    bool initialized_ = false;

    /// The graphics card that we'll end up selecting.
    VkPhysicalDevice physical_device_{};

    /// Logical device.
    VkDevice device_{};

    VkInstance instance_{};

    VkDebugUtilsMessengerEXT debug_messenger_{};

    // See https://developer.android.com/ndk/guides/graphics/validation-layer for enabling validation layer on Android.
    static constexpr bool enable_validation_layers_ =
#ifdef PATHFINDER_DEBUG
        true;
#else
        false;
#endif

    VkQueue graphics_queue_{};
    VkQueue present_queue_{};

    VkCommandPool command_pool_{};

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                         const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                         void *user_data) {
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            Logger::error(callback_data->pMessage);
        } else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            Logger::warn(callback_data->pMessage);
        } else {
            Logger::info(callback_data->pMessage);
        }

        return VK_FALSE;
    }

    static bool check_validation_layer_support();

    VkFormat find_depth_format() const;

    VkFormat find_supported_format(const std::vector<VkFormat> &candidates,
                                   VkImageTiling tiling,
                                   VkFormatFeatureFlags features) const;

    static std::vector<const char *> get_required_instance_extensions();

    /// Should be called every time a surface is (re)created.
    void initialize_after_surface_creation(VkSurfaceKHR surface);

    void setup_debug_messenger();

    void create_instance();

    bool check_device_extension_support(VkPhysicalDevice physical_device) const;

    /// Check if a physical device is suitable for the target surface.
    bool is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const;

    void pick_physical_device(VkSurfaceKHR surface);

    void create_logical_device(VkSurfaceKHR surface);

    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);

    void create_command_pool(VkSurfaceKHR surface);
};

} // namespace Pathfinder

#endif // PATHFINDER_WINDOW_BUILDER_VK_H
