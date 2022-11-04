#ifndef PATHFINDER_GPU_VK_DEBUG_MARKER_H
#define PATHFINDER_GPU_VK_DEBUG_MARKER_H

// Setup and functions for the VK_EXT_debug_marker_extension
// Extension spec can be found at
// https://github.com/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_marker/doc/specs/vulkan/appendices/VK_EXT_debug_marker.txt
// Note that the extension will only be present if run from an offline debugging application

#include <iostream>
#include <vector>

#include "../../common/color.h"
#include "../../common/logger.h"
#include "data.h"

namespace Pathfinder {

class DebugMarker {
private:
    bool active = false;
    bool extensionPresent = false;

    PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag = VK_NULL_HANDLE;
    PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName = VK_NULL_HANDLE;
    PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin = VK_NULL_HANDLE;
    PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd = VK_NULL_HANDLE;
    PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert = VK_NULL_HANDLE;

public:
    static DebugMarker* getSingleton() {
        static DebugMarker singleton;
        return &singleton;
    }

    // Sets the debug name of an object.
    // All Objects in Vulkan are represented by their 64-bit handles which are passed into this function
    // along with the object type.
    void setObjectName(VkDevice device, uint64_t object, VkDebugReportObjectTypeEXT objectType, const char* name) {
        // Check for valid function pointer (may not be present if not running in a debugging application).
        if (active) {
            VkDebugMarkerObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = objectType;
            nameInfo.object = object;
            nameInfo.pObjectName = name;
            vkDebugMarkerSetObjectName(device, &nameInfo);
        }
    }

    // Set the tag for an object.
    void setObjectTag(VkDevice device,
                      uint64_t object,
                      VkDebugReportObjectTypeEXT objectType,
                      uint64_t name,
                      size_t tagSize,
                      const void* tag) {
        // Check for valid function pointer (may not be present if not running in a debugging application).
        if (active) {
            VkDebugMarkerObjectTagInfoEXT tagInfo = {};
            tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
            tagInfo.objectType = objectType;
            tagInfo.object = object;
            tagInfo.tagName = name;
            tagInfo.tagSize = tagSize;
            tagInfo.pTag = tag;
            vkDebugMarkerSetObjectTag(device, &tagInfo);
        }
    }

    // Start a new debug marker region.
    void beginRegion(VkCommandBuffer cmdbuffer, const char* pMarkerName, ColorF color) {
        // Check for valid function pointer (may not be present if not running in a debugging application)
        if (active) {
            VkDebugMarkerMarkerInfoEXT markerInfo = {};
            markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            memcpy(markerInfo.color, &color, sizeof(float) * 4);
            markerInfo.pMarkerName = pMarkerName;
            vkCmdDebugMarkerBegin(cmdbuffer, &markerInfo);
        }
    }

    // Insert a new debug marker into the command buffer.
    void insert(VkCommandBuffer cmdbuffer, std::string markerName, ColorF color) {
        // Check for valid function pointer (may not be present if not running in a debugging application)
        if (active) {
            VkDebugMarkerMarkerInfoEXT markerInfo = {};
            markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            memcpy(markerInfo.color, &color, sizeof(float) * 4);
            markerInfo.pMarkerName = markerName.c_str();
            vkCmdDebugMarkerInsert(cmdbuffer, &markerInfo);
        }
    }

    // End the current debug marker region.
    void endRegion(VkCommandBuffer cmdBuffer) {
        // Check for valid function (may not be present if not running in a debugging application).
        if (vkCmdDebugMarkerEnd) {
            vkCmdDebugMarkerEnd(cmdBuffer);
        }
    }

    // Get function pointers for the debug report extensions from the device.
    void setup(VkDevice device, VkPhysicalDevice physicalDevice) {
        // Check if the debug marker extension is present (which is the case if run from a graphics debugger).
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

        for (auto extension : extensions) {
            if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
                extensionPresent = true;
                Logger::warn("extensionPresent", "Debug Marker");
                break;
            }
        }

        if (extensionPresent) {
            // The debug marker extension is not part of the core, so function pointers need to be loaded manually.
            vkDebugMarkerSetObjectTag =
                (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
            vkDebugMarkerSetObjectName =
                (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
            vkCmdDebugMarkerBegin =
                (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
            vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
            vkCmdDebugMarkerInsert =
                (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
            Logger::warn("active", "Debug Marker");
            // Set flag if at least one function pointer is present.
            active = (vkDebugMarkerSetObjectName != VK_NULL_HANDLE);

            if (active) {
                Logger::warn("Debug markers are enabled.", "Debug Marker");
            } else {
                Logger::warn("VK_EXT_debug_marker exists but the function pointers cannot be found!", "Debug Marker");
            }
        } else {
            Logger::warn(
                "VK_EXT_debug_marker is not present, debug markers are disabled. Try running from inside a Vulkan "
                "graphics debugger (e.g. RenderDoc).",
                "Debug Marker");
        }
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_VK_DEBUG_MARKER_H
