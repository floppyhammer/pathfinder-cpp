#ifndef PATHFINDER_GPU_PLATFORM_VK_H
#define PATHFINDER_GPU_PLATFORM_VK_H

#include "../../common/global_macros.h"
#include "../platform.h"
#include "driver.h"

#include <vector>
#include <iostream>
#include <optional>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    /// How many frames should be processed concurrently.
    const int MAX_FRAMES_IN_FLIGHT = 2;

    /// List of required validation layers.
    const std::vector<const char *> VALIDATION_LAYERS = {
            "VK_LAYER_KHRONOS_validation"
    };

    /// List of required device extensions.
    const std::vector<const char *> DEVICE_EXTENSIONS = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        [[nodiscard]] bool is_complete() const {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    class PlatformVk : public Platform {
    public:
        explicit PlatformVk(uint32_t window_width, uint32_t window_height);

        std::shared_ptr<Driver> create_driver() override;

        std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) override;

        static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
            auto platform = reinterpret_cast<PlatformVk *>(glfwGetWindowUserPointer(window));
            platform->framebuffer_resized = true;
        }

        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

        VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);

        VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes);

        static std::vector<const char *> get_required_extensions() {
            uint32_t glfw_extension_count = 0;
            const char **glfw_extensions;
            glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

            if (enable_validation_layers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                             VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                             const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                             void *user_data) {
            std::cerr << "Validation layer: " << callback_data->pMessage << std::endl;

            return VK_FALSE;
        }

        static bool check_validation_layer_support() {
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            std::vector<VkLayerProperties> available_layers(layer_count);
            vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

            for (const char *layer_name: VALIDATION_LAYERS) {
                bool layer_found = false;

                for (const auto &layer_properties: available_layers) {
                    if (strcmp(layer_name, layer_properties.layerName) == 0) {
                        layer_found = true;
                        break;
                    }
                }

                if (!layer_found) {
                    return false;
                }
            }

            return true;
        }

        QueueFamilyIndices find_queue_families(VkPhysicalDevice p_physical_device) const;

        SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice p_physical_device) const;

        [[nodiscard]] VkFormat find_depth_format() const;

        [[nodiscard]] VkFormat find_supported_format(const std::vector<VkFormat> &candidates,
                                                     VkImageTiling tiling,
                                                     VkFormatFeatureFlags features) const;

        void cleanup() override;

    public:
        VkSurfaceKHR surface{};

        /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
        VkPhysicalDevice physical_device{};

        /// Logical device.
        VkDevice device{};

        bool framebuffer_resized = false;

    private:
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};

        static const bool enable_validation_layers = true;

        VkQueue graphics_queue{};
        VkQueue present_queue{};

        VkCommandPool command_pool{};

    private:
        void init_window();

        void setup_debug_messenger();

        void create_instance();

        void create_surface();

        bool check_device_extension_support(VkPhysicalDevice p_physical_device) const;

        /**
         * Check if a physical device is suitable.
         * @param pPhysicalDevice
         * @return
         */
        bool is_device_suitable(VkPhysicalDevice p_physical_device) const;

        void pick_physical_device();

        void create_logical_device();

        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);

        void create_command_pool();
    };
}

#endif

#endif //PATHFINDER_GPU_PLATFORM_VK_H
