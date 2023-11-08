#ifndef PATHFINDER_GPU_VK_DEBUG_MARKER_H
#define PATHFINDER_GPU_VK_DEBUG_MARKER_H

// Setup and functions for the VK_EXT_debug_marker_extension
// Extension spec can be found at
// https://github.com/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_marker/doc/specs/vulkan/appendices/VK_EXT_debug_marker.txt
// Note that the extension will only be present if run from an offline debugging application

#include <cstring>
#include <iostream>
#include <vector>

#include "../../common/color.h"
#include "../../common/logger.h"
#include "base.h"

namespace Pathfinder {

class DebugMarker {
private:
    bool active = false;

    PFN_vkSetDebugUtilsObjectTagEXT vkDebugUtilsSetObjectTag = VK_NULL_HANDLE;
    PFN_vkSetDebugUtilsObjectNameEXT vkDebugUtilsSetObjectName = VK_NULL_HANDLE;
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel = VK_NULL_HANDLE;
    PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabel = VK_NULL_HANDLE;
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel = VK_NULL_HANDLE;

public:
    static DebugMarker* get_singleton() {
        static DebugMarker singleton;
        return &singleton;
    }

    /// Set the tag for an object.
    void set_object_tag(VkDevice device,
                        uint64_t object,
                        VkObjectType object_type,
                        uint64_t name,
                        size_t tag_size,
                        const void* tag) {
        // Check for valid function pointer (may not be present if not running in a debugging application).
        if (active) {
            VkDebugUtilsObjectTagInfoEXT tag_info =
                {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT, nullptr, object_type, object, name, tag_size, tag};

            vkDebugUtilsSetObjectTag(device, &tag_info);
        }
    }

    /// Sets the debug name of an object.
    /// All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
    /// along with the object type.
    void set_object_name(VkDevice device, uint64_t object, VkObjectType object_type, const std::string& name) {
        // Check for valid function pointer (may not be present if not running in a debugging application).
        if (active && !name.empty()) {
            VkDebugUtilsObjectNameInfoEXT nameInfo = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, // sType
                nullptr,                                            // pNext
                object_type,                                        // objectType
                object,                                             // objectHandle
                name.c_str(),                                       // pObjectName
            };

            vkDebugUtilsSetObjectName(device, &nameInfo);
        }
    }

    /// Start a new debug marker region.
    /// NOTE: Must be called after the command buffer begins recording.
    void begin_region(VkCommandBuffer cmd_buffer, const std::string& label_name, ColorF color) {
        // Check for valid function pointer (may not be present if not running in a debugging application)
        if (active && !label_name.empty()) {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                nullptr,
                label_name.c_str(),
            };
            memcpy(label_info.color, &color, sizeof(float) * 4);
            vkCmdBeginDebugUtilsLabel(cmd_buffer, &label_info);
        }
    }

    /// Insert a new debug marker into the command buffer.
    void insert(VkCommandBuffer cmd_buffer, const std::string& label_name, ColorF color) {
        // Check for valid function pointer (may not be present if not running in a debugging application)
        if (active && !label_name.empty()) {
            VkDebugUtilsLabelEXT label_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                nullptr,
                label_name.c_str(),
            };
            memcpy(label_info.color, &color, sizeof(float) * 4);
            vkCmdInsertDebugUtilsLabel(cmd_buffer, &label_info);
        }
    }

    /// End the current debug marker region.
    /// NOTE: Must be called before the command buffer ends recording.
    void end_region(VkCommandBuffer cmd_buffer) {
        // Check for valid function (may not be present if not running in a debugging application).
        if (active) {
            vkCmdEndDebugUtilsLabel(cmd_buffer);
        }
    }

    /// Get function pointers for the debug report extensions from the device.
    void setup(VkInstance instance) {
        // The debug marker extension is not part of the core, so function pointers need to be loaded manually.
        vkDebugUtilsSetObjectTag =
            (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT");
        vkDebugUtilsSetObjectName =
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
        vkCmdBeginDebugUtilsLabel =
            (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
        vkCmdInsertDebugUtilsLabel =
            (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
        vkCmdEndDebugUtilsLabel =
            (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");

        // Set flag if at least one function pointer is present.
        active = (vkDebugUtilsSetObjectTag != VK_NULL_HANDLE);

        if (!active) {
            Logger::warn("Debug markers disabled. Try running from inside a Vulkan graphics debugger (e.g. RenderDoc).",
                         "DebugMarker");
        }
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_VK_DEBUG_MARKER_H
