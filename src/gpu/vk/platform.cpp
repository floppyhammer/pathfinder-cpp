#include "platform.h"

#include "driver.h"
#include "swap_chain.h"

#include <stdexcept>
#include <set>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    std::shared_ptr<Platform> Platform::new_impl(DeviceType device_type,
                                                 uint32_t window_width,
                                                 uint32_t window_height) {
        if (device_type == DeviceType::Vulkan) {
            return std::make_shared<PlatformVk>(window_width, window_height);
        }

        abort();
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                               "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    PlatformVk::PlatformVk(uint32_t window_width, uint32_t window_height) {
        // Get a GLFW window.
        initWindow(window_width, window_height);

        // Initialize the Vulkan library by creating an instance.
        createInstance();

        setupDebugMessenger();

        // Create a GLFW window surface.
        createSurface();

        // Pick a suitable GPU.
        pickPhysicalDevice();

        // Create a logical device.
        createLogicalDevice();

        createCommandPool();
    }

    std::shared_ptr<Driver> PlatformVk::create_driver() {
        return std::make_shared<DriverVk>(device, physicalDevice, graphicsQueue, presentQueue, commandPool);
    }

    void PlatformVk::createCommandPool() {
        QueueFamilyIndices qfIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = qfIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // So we can reset command buffers.

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void PlatformVk::initWindow(uint32_t p_width, uint32_t p_height) {
        // Initializes the GLFW library.
        glfwInit();

        // To not create an OpenGL context (as we're using Vulkan).
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Enable window resizing. This needs us to recreate the swap chain.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Hide window upon creation as we need to center the window before showing it.
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        // Get monitor position (used to correctly center the window in a multi-monitor scenario).
        int monitors_count, monitor_x, monitor_y;
        GLFWmonitor **monitors = glfwGetMonitors(&monitors_count);
        const GLFWvidmode *videoMode = glfwGetVideoMode(monitors[0]);
        glfwGetMonitorPos(monitors[0], &monitor_x, &monitor_y);

        // Get DPI scale.
        float dpi_scale_x, dpi_scale_y;
        glfwGetMonitorContentScale(monitors[0], &dpi_scale_x, &dpi_scale_y);

        window = glfwCreateWindow(p_width, p_height, "Pathfinder Demo (Vulkan)", nullptr, nullptr);

        // Center window.
        glfwSetWindowPos(window,
                         monitor_x + (videoMode->width - p_width) / 2,
                         monitor_y + (videoMode->height - p_height) / 2);

        // Show window.
        glfwShowWindow(window);

        // Assign this to window user, so we can fetch it when window size changes.
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void PlatformVk::createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        // Structure specifying application information.
        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Simple Vulkan Renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine"; // Name of the engine (if any) used to create the application.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        appInfo.pNext = nullptr;

        // Structure specifying parameters of a newly created instance.
        VkInstanceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.flags = 0;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo; // Pointer to a structure extending this structure.
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        // Create a new Vulkan instance.
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance!");
        }
    }

    VkExtent2D PlatformVk::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width,
                                          std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height,
                                           std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    void PlatformVk::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void PlatformVk::setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void PlatformVk::createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    bool PlatformVk::checkDeviceExtensionSupport(VkPhysicalDevice pPhysicalDevice) const {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(pPhysicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(pPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapChainSupportDetails PlatformVk::querySwapChainSupport(VkPhysicalDevice pPhysicalDevice) const {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pPhysicalDevice, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(pPhysicalDevice, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevice, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(pPhysicalDevice,
                                                      surface,
                                                      &presentModeCount,
                                                      details.presentModes.data());
        }

        return details;
    }

    bool PlatformVk::isDeviceSuitable(VkPhysicalDevice pPhysicalDevice) const {
        QueueFamilyIndices indices = findQueueFamilies(pPhysicalDevice);

        bool extensionsSupported = checkDeviceExtensionSupport(pPhysicalDevice);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pPhysicalDevice);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(pPhysicalDevice, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    void PlatformVk::pickPhysicalDevice() {
        // Enumerates the physical devices accessible to a Vulkan instance.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        // Call again to get the physical devices.
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Pick a suitable one among the physical devices.
        for (const auto &d: devices) {
            if (isDeviceSuitable(d)) {
                physicalDevice = d;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    VkSurfaceFormatKHR PlatformVk::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR PlatformVk::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode: availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    QueueFamilyIndices PlatformVk::findQueueFamilies(VkPhysicalDevice pPhysicalDevice) const {
        QueueFamilyIndices qfIndices;

        // Reports properties of the queues of the specified physical device.
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyCount, nullptr);

        // Structure providing information about a queue family.
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pPhysicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily: queueFamilies) {
            // queueFlags is a bitmask of VkQueueFlagBits indicating capabilities of the queues in this queue family.
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                qfIndices.graphicsFamily = i;
            }

            // Query if presentation is supported.
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(pPhysicalDevice, i, surface, &presentSupport);

            if (presentSupport) {
                qfIndices.presentFamily = i;
            }

            // If both graphics family and present family acquired.
            if (qfIndices.isComplete()) {
                break;
            }

            i++;
        }

        return qfIndices;
    }

    void PlatformVk::createLogicalDevice() {
        QueueFamilyIndices qfIndices = findQueueFamilies(physicalDevice);

        // Structure specifying parameters of a newly created device queue.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::set<uint32_t> uniqueQueueFamilies = {qfIndices.graphicsFamily.value(), qfIndices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Structure describing the fine-grained features that can be supported by an implementation.
        VkPhysicalDeviceFeatures deviceFeatures{};
        // Specifies whether anisotropic filtering is supported.
        // If this feature is not enabled, the anisotropyEnable member of the VkSamplerCreateInfo structure must be VK_FALSE.
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // enabledLayerCount and ppEnabledLayerNames are deprecated and ignored.
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // A logical device is created as a connection to a physical device.
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        // Get a queue handle from a device.
        vkGetDeviceQueue(device, qfIndices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, qfIndices.presentFamily.value(), 0, &presentQueue);
    }

    VkFormat PlatformVk::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                             VkImageTiling tiling,
                                             VkFormatFeatureFlags features) const {
        for (VkFormat format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    VkFormat PlatformVk::findDepthFormat() const {
        return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void PlatformVk::cleanup() {
        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    std::shared_ptr<SwapChain> PlatformVk::create_swap_chain(const std::shared_ptr<Driver> &driver,
                                                             uint32_t width,
                                                             uint32_t height) {
        auto driver_vk = static_cast<DriverVk *>(driver.get());
        auto swap_chain_vk = std::make_shared<SwapChainVk>(width,
                                                           height,
                                                           this,
                                                           driver_vk);
        return swap_chain_vk;
    }
}

#endif
