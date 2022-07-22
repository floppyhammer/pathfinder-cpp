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
    const std::vector<const char *> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
    };

    /// List of required device extensions.
    const std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class PlatformVk : public Platform {
    public:
        static std::shared_ptr<Platform> create(uint32_t p_width, uint32_t p_height) {
            auto platform_vk = std::make_shared<PlatformVk>();
            platform_vk->init(p_width, p_height);
            return platform_vk;
        }

        void init(uint32_t window_width, uint32_t window_height);

        std::shared_ptr<Driver> create_driver() override;

        VkSurfaceKHR surface{};

        /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        /// Logical device.
        VkDevice device{};

        bool framebufferResized = false;

        std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver>& driver, uint32_t p_width, uint32_t p_height) override;

        static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
            auto platform = reinterpret_cast<PlatformVk *>(glfwGetWindowUserPointer(window));
            platform->framebufferResized = true;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        static std::vector<const char *> getRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char **glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                            void *pUserData) {
            std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        }

        static bool checkValidationLayerSupport() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char *layerName: validationLayers) {
                bool layerFound = false;

                for (const auto &layerProperties: availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pPhysicalDevice) const;

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice pPhysicalDevice) const;

        [[nodiscard]] VkFormat findDepthFormat() const;

        [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                                                   VkImageTiling tiling,
                                                   VkFormatFeatureFlags features) const;

        void cleanup() override;

    private:
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debugMessenger{};

        static const bool enableValidationLayers = true;

        VkQueue graphicsQueue{};
        VkQueue presentQueue{};

        VkCommandPool commandPool{};

    private:
        void initWindow(uint32_t window_width, uint32_t window_height);

        void setupDebugMessenger();

        void createInstance();

        void createSurface();

        bool checkDeviceExtensionSupport(VkPhysicalDevice pPhysicalDevice) const;

        /**
         * Check if a physical device is suitable.
         * @param pPhysicalDevice
         * @return
         */
        bool isDeviceSuitable(VkPhysicalDevice pPhysicalDevice) const;

        void pickPhysicalDevice();

        void createLogicalDevice();

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        void createCommandPool();
    };
}

#endif

#endif //PATHFINDER_GPU_PLATFORM_VK_H
