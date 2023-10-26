#include "window_builder.h"

#include <set>

#include "../../common/logger.h"
#include "../window_builder.h"
#include "debug_marker.h"
#include "queue.h"
#include "window.h"

#if (defined(WIN32) || defined(__linux__) || defined(__APPLE__)) && !defined(ANDROID)

namespace Pathfinder {

VkResult create_debug_utils_messenger_ext(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *create_info,
                                          const VkAllocationCallbacks *allocator,
                                          VkDebugUtilsMessengerEXT *debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroy_debug_utils_messenger_ext(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debug_messenger,
                                       const VkAllocationCallbacks *allocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, allocator);
    }
}

std::shared_ptr<WindowBuilder> WindowBuilder::new_impl(const Vec2I &size) {
    return std::make_shared<WindowBuilderVk>(size);
}

WindowBuilderVk::WindowBuilderVk(const Vec2I &size) {
    glfwInit();

    create_instance();

    setup_debug_messenger();

    // To not create an OpenGL context (as we're using Vulkan).
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto glfw_window = common_glfw_window_init(size, "Main");

    VkSurfaceKHR surface{};
    if (glfwCreateWindowSurface(instance, glfw_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    initialize_after_surface_creation(surface);

    auto window_vk = std::make_shared<WindowVk>(size, glfw_window, surface);

    main_window = window_vk;

    create_graphics_queues(window_vk->surface, window_vk->present_queue);
}

WindowBuilderVk::~WindowBuilderVk() {
    vkDestroyCommandPool(vk_device, command_pool, nullptr);

    // Destroy the logical device.
    vkDestroyDevice(vk_device, nullptr);

    if (enable_validation_layers) {
        destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
    }

    for (auto &w : sub_windows) {
        if (!w.expired()) {
            destroy_window(w.lock());
        }
    }
    sub_windows.clear();

    destroy_window(main_window);
    main_window.reset();

    // FIXME: we might need to handle the main window destruction more carefully.
    //    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwTerminate();
}

std::shared_ptr<Window> WindowBuilderVk::create_window(const Vec2I &size, const std::string &title) {
    auto glfw_window = common_glfw_window_init(size, title);

    VkSurfaceKHR surface{};
    if (glfwCreateWindowSurface(instance, glfw_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
    initialize_after_surface_creation(surface);

    auto new_window = std::make_shared<WindowVk>(size, glfw_window, surface);

    create_graphics_queues(surface, new_window->present_queue);

    sub_windows.push_back(new_window);

    return new_window;
}

void WindowBuilderVk::initialize_after_surface_creation(VkSurfaceKHR surface) {
    if (initialized) {
        return;
    }

    // Pick a suitable GPU.
    pick_physical_device(surface);

    // Create a logical device.
    create_logical_device(surface);

    create_command_pool(surface);

    create_graphics_queues(surface, graphics_queue);

    initialized = true;
}

void WindowBuilderVk::create_graphics_queues(VkSurfaceKHR surface, VkQueue &graphics_queue) {
    QueueFamilyIndices qf_indices = find_queue_families(physical_device, surface);

    // Get a queue handle from a device.
    vkGetDeviceQueue(vk_device, *qf_indices.graphics_family, 0, &graphics_queue);
}

std::shared_ptr<Device> WindowBuilderVk::request_device() {
    auto device = std::make_shared<DeviceVk>(vk_device, physical_device, graphics_queue, command_pool);
    return device;
}

std::shared_ptr<Queue> WindowBuilderVk::create_queue() {
    auto queue = std::make_shared<QueueVk>(vk_device, graphics_queue, present_queue);
    return queue;
}

void WindowBuilderVk::create_command_pool(VkSurfaceKHR surface) {
    QueueFamilyIndices qf_indices = find_queue_families(physical_device, surface);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = *qf_indices.graphics_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // So we can reset command buffers.

    if (vkCreateCommandPool(vk_device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void WindowBuilderVk::create_instance() {
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

        // Pointer to a structure extending this structure.
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;

        create_info.pNext = nullptr;
    }

    // Create a new Vulkan instance.
    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Vulkan instance!");
    }

    DebugMarker::get_singleton()->setup(instance);
}

void WindowBuilderVk::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info) {
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
}

void WindowBuilderVk::setup_debug_messenger() {
    if (!enable_validation_layers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info;
    populate_debug_messenger_create_info(create_info);

    if (create_debug_utils_messenger_ext(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void WindowBuilderVk::create_surface() {}

bool WindowBuilderVk::check_device_extension_support(VkPhysicalDevice _physical_device) const {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto &extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice _physical_device, VkSurfaceKHR surface) {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physical_device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  details.present_modes.data());
    }

    return details;
}

bool WindowBuilderVk::is_device_suitable(VkPhysicalDevice _physical_device, VkSurfaceKHR surface) const {
    QueueFamilyIndices indices = find_queue_families(_physical_device, surface);

    bool extensions_supported = check_device_extension_support(_physical_device);

    bool swapchain_adequate = false;
    if (extensions_supported) {
        SwapchainSupportDetails swapChainSupport = query_swapchain_support(_physical_device, surface);
        swapchain_adequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(_physical_device, &supported_features);

    return indices.is_complete() && extensions_supported && swapchain_adequate && supported_features.samplerAnisotropy;
}

void WindowBuilderVk::pick_physical_device(VkSurfaceKHR surface) {
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
    for (const auto &d : devices) {
        if (is_device_suitable(d, surface)) {
            physical_device = d;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (const auto &available_format : available_formats) {
        if (available_format.format == VK_FORMAT_R8G8B8A8_UNORM &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes) {
    for (const auto &available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

QueueFamilyIndices find_queue_families(VkPhysicalDevice _physical_device, VkSurfaceKHR surface) {
    QueueFamilyIndices qf_indices;

    // Reports properties of the queues of the specified physical device.
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, nullptr);

    // Structure providing information about a queue family.
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto &queue_family : queue_families) {
        // queueFlags is a bitmask of VkQueueFlagBits indicating capabilities of the queues in this queue family.
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            qf_indices.graphics_family = std::make_shared<uint32_t>(i);
        }

        // Query if presentation is supported.
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physical_device, i, surface, &present_support);

        if (present_support) {
            qf_indices.present_family = std::make_shared<uint32_t>(i);
        }

        // If both graphics family and present family acquired.
        if (qf_indices.is_complete()) {
            break;
        }

        i++;
    }

    return qf_indices;
}

std::vector<const char *> WindowBuilderVk::get_required_extensions() {
    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    if (enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool WindowBuilderVk::check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char *layer_name : VALIDATION_LAYERS) {
        bool layer_found = false;

        for (const auto &layer_properties : available_layers) {
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

void WindowBuilderVk::create_logical_device(VkSurfaceKHR surface) {
    QueueFamilyIndices qf_indices = find_queue_families(physical_device, surface);

    // Structure specifying parameters of a newly created device queue.
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    std::set<uint32_t> unique_queue_families = {*qf_indices.graphics_family, *qf_indices.present_family};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
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
    // If this feature is not enabled, the anisotropyEnable member of the VkSamplerCreateInfo structure must be
    // VK_FALSE.
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
    if (vkCreateDevice(physical_device, &create_info, nullptr, &vk_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    // Get a queue handle from a device.
    vkGetDeviceQueue(vk_device, *qf_indices.graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(vk_device, *qf_indices.present_family, 0, &present_queue);
}

VkFormat WindowBuilderVk::find_supported_format(const std::vector<VkFormat> &candidates,
                                                VkImageTiling tiling,
                                                VkFormatFeatureFlags features) const {
    for (VkFormat format : candidates) {
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

VkFormat WindowBuilderVk::find_depth_format() const {
    return find_supported_format({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void WindowBuilderVk::destroy_window(const std::shared_ptr<Window> &window) {
    auto window_vk = (WindowVk *)window.get();

    vkDestroySurfaceKHR(instance, window_vk->surface, nullptr);
    window_vk->surface = nullptr;

    glfwDestroyWindow(window_vk->glfw_window);
    window_vk->glfw_window = nullptr;
}

#endif

} // namespace Pathfinder
