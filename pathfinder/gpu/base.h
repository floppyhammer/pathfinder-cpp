#ifndef PATHFINDER_GPU_DATA_H
#define PATHFINDER_GPU_DATA_H

#include <cstdint>
#include <cstdlib>

namespace Pathfinder {

enum class DataType {
    i8,
    u8,
    i16,
    u16,
    i32,
    u32,
    f16,
    f32,
};

/// Texture format in GPU memory.
enum class TextureFormat {
    Rgba8Unorm,
    Bgra8Unorm,
    Rgba8Srgb,
    Bgra8Srgb,
    Rgba16Float,
};

enum class TextureLayout {
    Undefined,
    /// Present.
    PresentSrc,
    /// Render target.
    ColorAttachment,
    /// Sampler.
    ShaderReadOnly,
    /// Data transfer.
    TransferSrc,
    TransferDst,
    /// Compute image.
    General,
};

enum class ShaderStage {
    Vertex,
    Fragment,
    Compute,
    VertexAndFragment,
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
    Vertex,
    Instance,
};

struct VertexInputAttributeDescription {
    uint32_t binding;
    uint8_t size; // Must be one of 1, 2, 3, 4.
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
};

enum class MemoryProperty {
    /// Visible: can be mapped for host access.
    /// Coherent: host cache management commands are not needed.
    HostVisibleAndCoherent,
    /// Most efficient for device access.
    DeviceLocal,
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
    ReadAndWrite,
};

/// Operation to perform to the output attachment at the start of a render pass.
enum class AttachmentLoadOp {
    Load,
    Clear,
};

inline uint32_t get_pixel_size(TextureFormat format) {
    switch (format) {
        case TextureFormat::Rgba8Unorm:
        case TextureFormat::Bgra8Unorm:
        case TextureFormat::Rgba8Srgb:
        case TextureFormat::Bgra8Srgb: {
            return 4;
        }
        case TextureFormat::Rgba16Float: {
            return 8;
        }
        default:
            abort();
    }
}

inline DataType texture_format_to_data_type(TextureFormat format) {
    switch (format) {
        case TextureFormat::Rgba8Unorm:
        case TextureFormat::Bgra8Unorm:
        case TextureFormat::Rgba8Srgb:
        case TextureFormat::Bgra8Srgb: {
            return DataType::u8;
        }
        case TextureFormat::Rgba16Float: {
            return DataType::f16;
        }
        default:
            abort();
    }
}

enum class SamplerAddressMode {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder, // Not available in GLES until 3.2.
};

enum class SamplerFilter {
    Nearest,
    Linear,
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DATA_H
