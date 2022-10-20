#ifndef PATHFINDER_GPU_DATA_H
#define PATHFINDER_GPU_DATA_H

#include <cstdint>

#ifndef __ANDROID__
    #ifdef PATHFINDER_USE_VULKAN
        // Vulkan headers.
        #define GLFW_INCLUDE_VULKAN
        #include <GLFW/glfw3.h>
    #else
        // Include OpenGL header via GLAD.
        #include <glad/glad.h>
        // Prevent the GLFW header from including the OpenGL header.
        #define GLFW_INCLUDE_NONE
        #include <GLFW/glfw3.h>
    #endif
#else
    #ifdef PATHFINDER_USE_VULKAN
        #include "vulkan_wrapper.h"
    #else
        #ifdef PATHFINDER_USE_D3D11
            #include <GLES3/gl31.h>
        #else
            #include <GLES3/gl3.h>
        #endif
    #endif
#endif

namespace Pathfinder {
enum class DataType {
    // Integers.
    BYTE,           // 1 byte
    UNSIGNED_BYTE,  // 1 byte
    SHORT,          // 2 bytes
    UNSIGNED_SHORT, // 2 bytes
    INT,            // 4 bytes
    UNSIGNED_INT,   // 4 bytes

    // Floats.
    FLOAT,      // 4 bytes
    HALF_FLOAT, // 2 bytes
};

/// Texture format in GPU memory.
enum class TextureFormat {
    RGBA8_UNORM,
    BGRA8_UNORM,
    RGBA8_SRGB,
    BGRA8_SRGB,
    RGBA16F,
};

enum class TextureLayout {
    UNDEFINED,
    PRESENT_SRC,
    COLOR_ATTACHMENT,
    SHADER_READ_ONLY,
    TRANSFER_SRC,
    TRANSFER_DST,
    GENERAL,
};

// TODO(floppyhammer): Make this bits.
enum class ShaderStage {
    Vertex,
    Fragment,
    VertexFragment,
    Compute,
    Max,
};

enum class DeviceType {
    OpenGl3, // Or ES 3.0
    OpenGl4, // Or ES 3.1
    Vulkan,
};

enum class BlendFactor {
    /// 1.0
    One,
    /// 1.0 - S.alpha
    OneMinusSrcAlpha,
};

enum class BlendOperation {
    /// Src + Dst
    Add,
};

struct BlendComponent {
    /// Multiplier for the source, which is produced by the fragment shader.
    BlendFactor src_factor;
    /// Multiplier for the destination, which is stored in the target.
    BlendFactor dst_factor;
    /// The binary operation applied to the source and destination,
    /// multiplied by their respective factors.
    BlendOperation operation;
};

struct BlendState {
    /// If blend is enabled.
    bool enabled;
    /// Color equation.
    BlendComponent color;
    /// Alpha equation.
    BlendComponent alpha;

    static BlendState from_over() {
        BlendState blend_state{};
        blend_state.enabled = true;
        blend_state.color.src_factor = BlendFactor::One;
        blend_state.color.dst_factor = BlendFactor::OneMinusSrcAlpha;
        blend_state.alpha.src_factor = BlendFactor::One;
        blend_state.alpha.dst_factor = BlendFactor::OneMinusSrcAlpha;
        blend_state.color.operation = BlendOperation::Add;

        return blend_state;
    }

    static BlendState from_equal() {
        BlendState blend_state{};
        blend_state.enabled = true;
        blend_state.color.src_factor = BlendFactor::One;
        blend_state.color.dst_factor = BlendFactor::One;
        blend_state.alpha.src_factor = BlendFactor::One;
        blend_state.alpha.dst_factor = BlendFactor::One;
        blend_state.color.operation = BlendOperation::Add;

        return blend_state;
    }
};

enum class VertexInputRate {
    VERTEX = 0,
    INSTANCE = 1,
};

struct VertexInputAttributeDescription {
    uint32_t binding;
    uint8_t size; // Must be 1, 2, 3, 4.
    DataType type;
    uint32_t stride;
    size_t offset;
    VertexInputRate vertex_input_rate;
};

enum class DescriptorType {
    UniformBuffer = 0,
    Sampler,
    StorageBuffer,
    Image,
    Max,
};

enum class MemoryProperty {
    HOST_VISIBLE_AND_COHERENT,
    DEVICE_LOCAL,
};

enum class BufferType {
    Vertex,
    Index,
    Uniform,
    Storage,
};

enum class StorageBufferUsage {
    Read,
    Write,
    ReadWrite,
};

enum class AttachmentLoadOp {
    LOAD = 0,
    CLEAR,
};

inline uint32_t get_pixel_size(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::BGRA8_UNORM:
        case TextureFormat::RGBA8_SRGB:
        case TextureFormat::BGRA8_SRGB: {
            return 4;
        }
        case TextureFormat::RGBA16F: {
            return 8;
        }
    }
}

inline DataType texture_format_to_data_type(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::BGRA8_UNORM:
        case TextureFormat::RGBA8_SRGB:
        case TextureFormat::BGRA8_SRGB: {
            return DataType::UNSIGNED_BYTE;
        }
        case TextureFormat::RGBA16F: {
            return DataType::HALF_FLOAT;
        }
    }
}
} // namespace Pathfinder

#endif // PATHFINDER_GPU_DATA_H
