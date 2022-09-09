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

    VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                              const VkDebugUtilsMessengerCreateInfoEXT *create_info,
                                              const VkAllocationCallbacks *allocator,
                                              VkDebugUtilsMessengerEXT *debug_messenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                               "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, create_info, allocator, debug_messenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void destroy_debug_utils_messenger_ext(VkInstance instance,
                                           VkDebugUtilsMessengerEXT debug_messenger,
                                           const VkAllocationCallbacks *allocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debug_messenger, allocator);
        }
    }

    PlatformVk::PlatformVk(uint32_t window_width, uint32_t window_height) {
        // Get a GLFW window.
        init_window(window_width, window_height);

        // Initialize the Vulkan library by creating an instance.
        create_instance();

        setup_debug_messenger();

        // Create a GLFW window surface.
        create_surface();

        // Pick a suitable GPU.
        pick_physical_device();

        // Create a logical device.
        create_logical_device();

        create_command_pool();
    }

    std::shared_ptr<Driver> PlatformVk::create_driver() {
        return std::make_shared<DriverVk>(device, physical_device, graphics_queue, present_queue, command_pool);
    }

    void PlatformVk::create_command_pool() {
        QueueFamilyIndices qf_indices = find_queue_families(physical_device);

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = qf_indices.graphics_family.value();
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // So we can reset command buffers.

        if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void PlatformVk::init_window(uint32_t p_width, uint32_t p_height) {
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
        glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
    }

    void PlatformVk::create_instance() {
        if (enable_validation_layers && !check_validation_layer_support()) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        // Structure specifying application information.
        VkApplicationInfo app_info;
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Simple Vulkan Renderer";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine"; // Name of the engine (if any) used to create the application.
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;
        app_info.pNext = nullptr;

        // Structure specifying parameters of a newly created instance.
        VkInstanceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.flags = 0;

        auto extensions = get_required_extensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
        if (enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

            populate_debug_messenger_create_info(debug_create_info);
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debug_create_info; // Pointer to a structure extending this structure.
        } else {
            create_info.enabledLayerCount = 0;

            create_info.pNext = nullptr;
        }

        // Create a new Vulkan instance.
        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance!");
        }
    }

    VkExtent2D PlatformVk::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actual_extent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actual_extent.width = std::max(capabilities.minImageExtent.width,
                                           std::min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height = std::max(capabilities.minImageExtent.height,
                                            std::min(capabilities.maxImageExtent.height, actual_extent.height));

            return actual_extent;
        }
    }

    void PlatformVk::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info) {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
    }

    void PlatformVk::setup_debug_messenger() {
        if (!enable_validation_layers) return;

        VkDebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);

        if (create_debug_utils_messenger_ext(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void PlatformVk::create_surface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    bool PlatformVk::check_device_extension_support(VkPhysicalDevice p_physical_device) const {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(p_physical_device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(p_physical_device, nullptr, &extension_count, available_extensions.data());

        std::set<std::string> required_extensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

        for (const auto &extension: available_extensions) {
            required_extensions.erase(extension.extensionName);
        }

        return required_extensions.empty();
    }

    SwapchainSupportDetails PlatformVk::query_swapchain_support(VkPhysicalDevice p_physical_device) const {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_physical_device, surface, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, surface, &format_count, nullptr);

        if (format_count != 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, surface, &format_count, details.formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device,
                                                      surface,
                                                      &present_mode_count,
                                                      details.present_modes.data());
        }

        return details;
    }

    bool PlatformVk::is_device_suitable(VkPhysicalDevice p_physical_device) const {
        QueueFamilyIndices indices = find_queue_families(p_physical_device);

        bool extensions_supported = check_device_extension_support(p_physical_device);

        bool swapchain_adequate = false;
        if (extensions_supported) {
            SwapchainSupportDetails swapChainSupport = query_swapchain_support(p_physical_device);
            swapchain_adequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
        }

        VkPhysicalDeviceFeatures supported_features;
        vkGetPhysicalDeviceFeatures(p_physical_device, &supported_features);

        return indices.is_complete() && extensions_supported && swapchain_adequate &&
               supported_features.samplerAnisotropy;
    }

    void PlatformVk::pick_physical_device() {
        // Enumerates the physical devices accessible to a Vulkan instance.
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

        if (device_count == 0) {
            throw std::runtime_error("Failed to find a GPU with Vulkan support!");
        }

        // Call again to get the physical devices.
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

        // Pick a suitable one among the physical devices.
        for (const auto &d: devices) {
            if (is_device_suitable(d)) {
                physical_device = d;
                break;
            }
        }

        if (physical_device == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    VkSurfaceFormatKHR
    PlatformVk::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats) {
        for (const auto &available_format: available_formats) {
            if (available_format.format == VK_FORMAT_R8G8B8A8_UNORM &&
                available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return available_format;
            }
        }

        return available_formats[0];
    }

    VkPresentModeKHR
    PlatformVk::choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes) {
        for (const auto &available_present_mode: available_present_modes) {
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return available_present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    QueueFamilyIndices PlatformVk::find_queue_families(VkPhysicalDevice p_physical_device) const {
        QueueFamilyIndices qf_indices;

        // Reports properties of the queues of the specified physical device.
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(p_physical_device, &queue_family_count, nullptr);

        // Structure providing information about a queue family.
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(p_physical_device, &queue_family_count, queue_families.data());

        int i = 0;
        for (const auto &queue_family: queue_families) {
            // queueFlags is a bitmask of VkQueueFlagBits indicating capabilities of the queues in this queue family.
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                qf_indices.graphics_family = i;
            }

            // Query if presentation is supported.
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(p_physical_device, i, surface, &present_support);

            if (present_support) {
                qf_indices.present_family = i;
            }

            // If both graphics family and present family acquired.
            if (qf_indices.is_complete()) {
                break;
            }

            i++;
        }

        return qf_indices;
    }

    void PlatformVk::create_logical_device() {
        QueueFamilyIndices qf_indices = find_queue_families(physical_device);

        // Structure specifying parameters of a newly created device queue.
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        std::set<uint32_t> unique_queue_families = {qf_indices.graphics_family.value(),
                                                    qf_indices.present_family.value()};

        float queue_priority = 1.0f;
        for (uint32_t queue_family: unique_queue_families) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        // Structure describing the fine-grained features that can be supported by an implementation.
        VkPhysicalDeviceFeatures device_features{};
        // Specifies whether anisotropic filtering is supported.
        // If this feature is not enabled, the anisotropyEnable member of the VkSamplerCreateInfo structure must be VK_FALSE.
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();

        create_info.pEnabledFeatures = &device_features;

        create_info.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
        create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

        // enabledLayerCount and ppEnabledLayerNames are deprecated and ignored.
        if (enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        } else {
            create_info.enabledLayerCount = 0;
        }

        // A logical device is created as a connection to a physical device.
        if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        // Get a queue handle from a device.
        vkGetDeviceQueue(device, qf_indices.graphics_family.value(), 0, &graphics_queue);
        vkGetDeviceQueue(device, qf_indices.present_family.value(), 0, &present_queue);
    }

    VkFormat PlatformVk::find_supported_format(const std::vector<VkFormat> &candidates,
                                               VkImageTiling tiling,
                                               VkFormatFeatureFlags features) const {
        for (VkFormat format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    VkFormat PlatformVk::find_depth_format() const {
        return find_supported_format(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void PlatformVk::cleanup() {
        vkDestroyCommandPool(device, command_pool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enable_validation_layers) {
            destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
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
