#ifndef PATHFINDER_GPU_DATA_H
#define PATHFINDER_GPU_DATA_H

#include <cstdint>

namespace Pathfinder {
    enum class DataType {
        // Integers.
        BYTE, // 1 byte
        UNSIGNED_BYTE, // 1 byte
        SHORT, // 2 bytes
        UNSIGNED_SHORT, // 2 bytes
        INT, // 4 bytes
        UNSIGNED_INT, // 4 bytes

        // Floats.
        FLOAT, // 4 bytes
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

    enum class ImageLayout {
        PRESENT_SRC,
        SHADER_READ_ONLY,
        TRANSFER_SRC,
        TRANSFER_DST,
    };

    /// Texture format in CPU memory.
    enum class PixelDataFormat {
        RED,
        RGBA,
    };

    enum class ShaderType {
        Vertex,
        Fragment,
        VertexFragment,
        Compute,
        Max,
    };

    enum class DeviceType {
        GL3, // Or ES 3.0
        GL4, // Or ES 3.1
        Vulkan,
    };

    enum class BlendFactor {
        ONE,
        ONE_MINUS_SRC_ALPHA,
    };

    struct ColorBlendState {
        bool blend_enable;
        BlendFactor src_blend_factor;
        BlendFactor dst_blend_factor;
    };

    enum class DescriptorType {
        UniformBuffer = 0,
        Texture,
        GeneralBuffer,
        Image,
        Max,
    };

    enum class MemoryProperty {
        HOST_VISIBLE_AND_COHERENT,
        DEVICE_LOCAL,
    };

    enum class BufferType {
        Vertex,
        Uniform,
        General,
    };

    enum class GeneralBufferUsage {
        Read,
        Write,
        ReadWrite,
    };

    enum class AttachmentLoadOp {
        LOAD = 0,
        CLEAR = 1,
    };

    inline uint32_t get_pixel_size(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8_UNORM:
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
}

#endif //PATHFINDER_GPU_DATA_H
