#include "window_builder.h"

#include <set>

#include "../window_builder.h"
#include "debug_marker.h"
#include "queue.h"
#include "window.h"

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
#ifndef __ANDROID__
    return std::make_shared<WindowBuilderVk>(size);
#else
    return nullptr;
#endif
}

#ifndef __ANDROID__
WindowBuilderVk::WindowBuilderVk(const Vec2I &size) {
    glfwInit();

    // To not create an OpenGL context (as we're using Vulkan).
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    create_instance();

    setup_debug_messenger();

    float dpi_scaling_factor;
    auto glfw_window = glfw_window_init(size, PRIMARY_WINDOW_TITLE, dpi_scaling_factor, false, nullptr);

    VkSurfaceKHR surface{};
    VK_CHECK_RESULT(glfwCreateWindowSurface(instance_, glfw_window, nullptr, &surface))

    initialize_after_surface_creation(surface);

    primary_window_ = std::make_shared<WindowVk>(size, glfw_window, surface, instance_);
    primary_window_->set_dpi_scaling_factor(dpi_scaling_factor);
}
#else
WindowBuilderVk::WindowBuilderVk(ANativeWindow *native_window, const Vec2I &window_size) {
    native_window_ = native_window;

    create_instance();

    setup_debug_messenger();

    VkSurfaceKHR surface{};
    VkAndroidSurfaceCreateInfoKHR create_info{.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
                                              .pNext = nullptr,
                                              .flags = 0,
                                              .window = native_window_};

    VK_CHECK_RESULT(vkCreateAndroidSurfaceKHR(instance_, &create_info, nullptr, &surface))

    initialize_after_surface_creation(surface);

    primary_window_ = std::make_shared<WindowVk>(window_size, surface, instance_);
}
#endif

WindowBuilderVk::~WindowBuilderVk() {
#ifndef __ANDROID__
    // Destroy windows.
    WindowBuilderVk::stop_and_destroy_swapchains();
#endif

    vkDestroyCommandPool(device_, command_pool_, nullptr);

    // Destroy the logical device.
    vkDestroyDevice(device_, nullptr);

    if (enable_validation_layers_) {
        destroy_debug_utils_messenger_ext(instance_, debug_messenger_, nullptr);
    }

    vkDestroyInstance(instance_, nullptr);

#ifndef __ANDROID__
    glfwTerminate();
#endif
}

void WindowBuilderVk::stop_and_destroy_swapchains() {
    for (auto &w : sub_windows_) {
        // We need to destroy a window explicitly in case its smart pointer is held elsewhere.
        w->destroy();
    }

    primary_window_->swapchain_->destroy();
}

uint8_t WindowBuilderVk::create_window(const Vec2I &size, const std::string &title) {
#ifndef __ANDROID__
    float dpi_scaling_factor;
    auto glfw_window = glfw_window_init(size, title, dpi_scaling_factor, false, nullptr);

    VkSurfaceKHR surface{};
    VK_CHECK_RESULT(glfwCreateWindowSurface(instance_, glfw_window, nullptr, &surface))

    auto new_window = std::make_shared<WindowVk>(size, glfw_window, surface, instance_);
    new_window->set_dpi_scaling_factor(dpi_scaling_factor);

    sub_windows_.push_back(new_window);

    new_window->window_index = sub_windows_.size();

    return sub_windows_.size();
#else
    return 0;
#endif
}

void WindowBuilderVk::initialize_after_surface_creation(VkSurfaceKHR surface) {
    if (initialized_) {
        return;
    }

    // Pick a suitable GPU.
    pick_physical_device(surface);

    // Create a logical device.
    create_logical_device(surface);

    create_command_pool(surface);

    create_queues(surface);

    initialized_ = true;
}

void WindowBuilderVk::create_queues(VkSurfaceKHR surface) {
    QueueFamilyIndices qf_indices = find_queue_families(physical_device_, surface);

    // Get a queue handle from a device.
    vkGetDeviceQueue(device_, *qf_indices.graphics_family, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, *qf_indices.present_family, 0, &present_queue_);
}

std::shared_ptr<Device> WindowBuilderVk::request_device() {
    auto device = std::shared_ptr<DeviceVk>(
        new DeviceVk(device_, physical_device_, graphics_queue_, present_queue_, command_pool_));
    return device;
}

std::shared_ptr<Queue> WindowBuilderVk::create_queue() {
    auto queue = std::shared_ptr<QueueVk>(new QueueVk(device_, graphics_queue_, present_queue_));
    return queue;
}

void WindowBuilderVk::create_command_pool(VkSurfaceKHR surface) {
    const QueueFamilyIndices qf_indices = find_queue_families(physical_device_, surface);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = *qf_indices.graphics_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // So we can reset command buffers.

    VK_CHECK_RESULT(vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_))
}

void WindowBuilderVk::create_instance() {
    if (enable_validation_layers_ && !check_validation_layer_support()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    // Structure specifying application information.
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Structure specifying parameters of a new instance.
    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

    // Required instance extensions.
    const auto extensions = get_required_instance_extensions();
    instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instance_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    if (enable_validation_layers_) {
        instance_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        instance_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        populate_debug_messenger_create_info(debug_create_info);

        instance_info.pNext = &debug_create_info;
    }

    VK_CHECK_RESULT(vkCreateInstance(&instance_info, nullptr, &instance_))

    DebugMarker::get_singleton()->setup(instance_);
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
    if (!enable_validation_layers_) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info;
    populate_debug_messenger_create_info(create_info);

    VK_CHECK_RESULT(create_debug_utils_messenger_ext(instance_, &create_info, nullptr, &debug_messenger_))
}

bool WindowBuilderVk::check_device_extension_support(VkPhysicalDevice physical_device) const {
    // Get available device extensions.
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

    // Check if the required extensions are available.
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
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            _physical_device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

bool WindowBuilderVk::is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const {
    QueueFamilyIndices qf_indices = find_queue_families(physical_device, surface);

    bool extensions_supported = check_device_extension_support(physical_device);

    bool swapchain_adequate = false;
    if (extensions_supported) {
        SwapchainSupportDetails swapChainSupport = query_swapchain_support(physical_device, surface);
        swapchain_adequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(physical_device, &supported_features);

    return qf_indices.is_complete() && extensions_supported && swapchain_adequate &&
           supported_features.samplerAnisotropy;
}

void WindowBuilderVk::pick_physical_device(VkSurfaceKHR surface) {
    // Ger the number of the physical devices accessible to a Vulkan instance.
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("Failed to find a GPU with Vulkan support!");
    }

    // Call again to get the physical devices.
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

    // Pick a suitable physical device for the target surface.
    for (const auto &d : devices) {
        if (is_device_suitable(d, surface)) {
            physical_device_ = d;
            break;
        }
    }

    if (physical_device_ == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to pick a suitable GPU!");
    }
}

VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (const auto &format : available_formats) {
        if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes) {
    for (const auto &present_mode : available_present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    QueueFamilyIndices qf_indices;

    // Reports properties of the queues of the specified physical device.
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    // Structure providing information about a queue family.
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto &queue_family : queue_families) {
        // queueFlags is a bitmask of VkQueueFlagBits indicating capabilities of the queues in this queue family.
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            qf_indices.graphics_family = std::make_shared<uint32_t>(i);
        }

        // Query if presentation is supported.
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);

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

std::vector<const char *> WindowBuilderVk::get_required_instance_extensions() {
#ifndef __ANDROID__
    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
#else
    std::vector<const char *> extensions = INSTANCE_EXTENSIONS;
#endif

    if (enable_validation_layers_) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool WindowBuilderVk::check_validation_layer_support() {
    // Get available layers.
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    // Check if our specified layers are among the available layers.
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
    QueueFamilyIndices qf_indices = find_queue_families(physical_device_, surface);

    std::set<uint32_t> unique_queue_families = {*qf_indices.graphics_family, *qf_indices.present_family};

    float queue_priority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    for (uint32_t queue_family : unique_queue_families) {
        // Structure specifying parameters of a newly created device queue.
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

    // Specify needed device extensions.
    create_info.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    // enabledLayerCount and ppEnabledLayerNames are deprecated and ignored.
    if (enable_validation_layers_) {
        create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    // A logical device is created as a connection to a physical device.
    VK_CHECK_RESULT(vkCreateDevice(physical_device_, &create_info, nullptr, &device_))

    // Get a queue handle from a device.
    vkGetDeviceQueue(device_, *qf_indices.graphics_family, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, *qf_indices.present_family, 0, &present_queue_);
}

VkFormat WindowBuilderVk::find_supported_format(const std::vector<VkFormat> &candidates,
                                                VkImageTiling tiling,
                                                VkFormatFeatureFlags features) const {
    for (const VkFormat &format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device_, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
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

VkPhysicalDevice WindowBuilderVk::get_physical_device() const {
    return physical_device_;
}

VkDevice WindowBuilderVk::get_device() const {
    return device_;
}

} // namespace Pathfinder
