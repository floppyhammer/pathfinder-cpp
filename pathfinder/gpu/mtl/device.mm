#include "device.h"

#include "../descriptor_set.h"
#include "base.h"
#include "buffer.h"
#include "command_encoder.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "queue.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "sampler.h"
#include "shader_module.h"
#include "texture.h"

namespace Pathfinder {

DeviceMtl::DeviceMtl(id<MTLDevice> device, id<MTLCommandQueue> mtl_cmd_queue, int frames_in_flight)
    : Device(frames_in_flight), mtl_device_(device), mtl_cmd_queue_(mtl_cmd_queue) {
    backend_type = BackendType::Metal;
    queue_ = std::shared_ptr<QueueMtl>(new QueueMtl(device, mtl_cmd_queue));
}

std::shared_ptr<Buffer> DeviceMtl::create_buffer(const BufferDescriptor &descriptor, const std::string &label) {
    auto mtl_buffer = [mtl_device_ newBufferWithLength:descriptor.size
                                               options:to_mtl_resource_option(descriptor.property)];

    auto buffer_mtl = std::shared_ptr<BufferMtl>(new BufferMtl(descriptor));
    buffer_mtl->mtl_buffer_ = mtl_buffer;
    return buffer_mtl;
}

std::shared_ptr<Texture> DeviceMtl::create_texture(const TextureDescriptor &descriptor, const std::string &label) {
    auto pixel_format = to_mtl_pixel_format(descriptor.format);

    MTLTextureDescriptor *mtl_texture_desc = [[MTLTextureDescriptor alloc] init];
    mtl_texture_desc.textureType = MTLTextureType2D;
    mtl_texture_desc.pixelFormat = pixel_format;
    mtl_texture_desc.width = descriptor.size.x;
    mtl_texture_desc.height = descriptor.size.y;
    mtl_texture_desc.mipmapLevelCount = 1;

    // Use Shared mode for textures that might be updated by CPU, Private for GPU-only.
    // In Pathfinder, many textures like Mask or Atlas are updated via write_texture.
    mtl_texture_desc.storageMode = MTLStorageModeShared;
    mtl_texture_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;

    auto mtl_texture = [mtl_device_ newTextureWithDescriptor:mtl_texture_desc];
    if (label.length() > 0) {
        mtl_texture.label = [NSString stringWithUTF8String:label.c_str()];
    }

    auto texture_mtl = std::shared_ptr<TextureMtl>(new TextureMtl(descriptor, label));
    texture_mtl->mtl_texture_ = mtl_texture;

    return texture_mtl;
}

std::shared_ptr<Sampler> DeviceMtl::create_sampler(SamplerDescriptor descriptor) {
    MTLSamplerDescriptor *mtl_sampler_desc = [MTLSamplerDescriptor new];
    mtl_sampler_desc.minFilter = to_mtl_sampler_filter(descriptor.min_filter);
    mtl_sampler_desc.magFilter = to_mtl_sampler_filter(descriptor.mag_filter);

    mtl_sampler_desc.sAddressMode = to_mtl_sampler_address_mode(descriptor.address_mode_u);
    mtl_sampler_desc.tAddressMode = to_mtl_sampler_address_mode(descriptor.address_mode_v);

    auto mtl_sampler = [mtl_device_ newSamplerStateWithDescriptor:mtl_sampler_desc];
    auto sampler_mtl = std::shared_ptr<SamplerMtl>(new SamplerMtl(descriptor));
    sampler_mtl->mtl_sampler_ = mtl_sampler;

    return sampler_mtl;
}

std::shared_ptr<RenderPass> DeviceMtl::create_render_pass(TextureFormat format,
                                                          AttachmentLoadOp load_op,
                                                          const std::string &label) {
    return std::shared_ptr<RenderPassMtl>(new RenderPassMtl(format, load_op, false, label));
}

std::shared_ptr<RenderPass> DeviceMtl::create_swap_chain_render_pass(TextureFormat format, AttachmentLoadOp load_op) {
    return std::shared_ptr<RenderPassMtl>(new RenderPassMtl(format, load_op, true, "SwapChain render pass"));
}

std::shared_ptr<DescriptorSetLayout> DeviceMtl::create_descriptor_set_layout(
    const std::vector<DescriptorLayout> &descriptors) {
    return std::shared_ptr<DescriptorSetLayout>(new DescriptorSetLayout(descriptors));
}

std::shared_ptr<DescriptorSet> DeviceMtl::create_descriptor_set(std::shared_ptr<DescriptorSetLayout> layout) {
    return std::shared_ptr<DescriptorSet>(new DescriptorSet(layout));
}

std::shared_ptr<RenderPipeline> DeviceMtl::create_render_pipeline(
    const std::shared_ptr<ShaderModule> &vert_shader_module,
    const std::shared_ptr<ShaderModule> &frag_shader_module,
    const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
    BlendState blend_state,
    const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout,
    TextureFormat target_format,
    const std::string &label) {
    // Vertex attributes.
    MTLVertexDescriptor *vertex_desc = [MTLVertexDescriptor new];
    uint32_t attribute_index = 0;
    for (auto &d : attribute_descriptions) {
        vertex_desc.attributes[attribute_index].format = to_mtl_vertex_format(d.type, d.size);
        vertex_desc.attributes[attribute_index].offset = d.offset;
        vertex_desc.attributes[attribute_index].bufferIndex = d.binding;

        vertex_desc.layouts[d.binding].stride = d.stride;
        vertex_desc.layouts[d.binding].stepFunction = d.vertex_input_rate == VertexInputRate::Vertex
                                                          ? MTLVertexStepFunction::MTLVertexStepFunctionPerVertex
                                                          : MTLVertexStepFunction::MTLVertexStepFunctionPerInstance;
        vertex_desc.layouts[d.binding].stepRate = 1;

        attribute_index++;
    }

    // Vertex and fragment shaders.
    MTLRenderPipelineDescriptor *pipeline_desc = [MTLRenderPipelineDescriptor new];
    pipeline_desc.vertexDescriptor = vertex_desc;
    pipeline_desc.label = [NSString stringWithUTF8String:label.c_str()];
    pipeline_desc.vertexFunction = std::static_pointer_cast<ShaderModuleMtl>(vert_shader_module)->function();
    pipeline_desc.fragmentFunction = std::static_pointer_cast<ShaderModuleMtl>(frag_shader_module)->function();

    // One attachment for now.
    pipeline_desc.colorAttachments[0].pixelFormat = to_mtl_pixel_format(target_format);
    pipeline_desc.colorAttachments[0].writeMask = MTLColorWriteMaskAll;

    if (blend_state.enabled) {
        pipeline_desc.colorAttachments[0].blendingEnabled = true;

        pipeline_desc.colorAttachments[0].sourceRGBBlendFactor = to_mtl_blend_factor(blend_state.color.src_factor);
        pipeline_desc.colorAttachments[0].destinationRGBBlendFactor = to_mtl_blend_factor(blend_state.color.dst_factor);
        pipeline_desc.colorAttachments[0].rgbBlendOperation = to_mtl_blend_op(BlendOperation::Add);

        pipeline_desc.colorAttachments[0].sourceAlphaBlendFactor = to_mtl_blend_factor(blend_state.alpha.src_factor);
        pipeline_desc.colorAttachments[0].destinationAlphaBlendFactor =
            to_mtl_blend_factor(blend_state.alpha.dst_factor);
        pipeline_desc.colorAttachments[0].alphaBlendOperation = to_mtl_blend_op(BlendOperation::Add);
    } else {
        pipeline_desc.colorAttachments[0].blendingEnabled = false;
    }

    NSError *err = nil;
    auto pipeline_state = [mtl_device_ newRenderPipelineStateWithDescriptor:pipeline_desc error:&err];
    if (pipeline_state == nil) {
        NSLog(@"Failed to create pipeline state: %@", [err localizedDescription]);
        return nullptr;
    }

    auto render_pipeline_mtl =
        std::shared_ptr<RenderPipelineMtl>(new RenderPipelineMtl(attribute_descriptions, blend_state, label));
    render_pipeline_mtl->pipeline_state_ = pipeline_state;

    return render_pipeline_mtl;
}

std::shared_ptr<ShaderModule> DeviceMtl::create_shader_module(const std::shared_ptr<Shader> &shader,
                                                              const std::string &label) {
    auto code = shader->get_shader_code({ShaderSourceType::MSL, 1, 2});
    if (!code) {
        Logger::error("Failed to find a compatible MSL variant in the provided shader binary!");
        return nullptr;
    }

    std::shared_ptr<ShaderModuleMtl> shader_module(new ShaderModuleMtl);

    NSString *code_str = [NSString stringWithUTF8String:code->code.data()];

    NSError *error = nil;
    shader_module->library_ = [mtl_device_ newLibraryWithSource:code_str options:nil error:nil];
    if (error) {
        NSLog(@"Library compilation error: %@", error.localizedDescription);
        return nullptr;
    }

    // NSLog(@"Available functions: %@", shader_module->library_.functionNames);

    NSString *entry_point = [NSString stringWithUTF8String:code->entry_point.c_str()];
    shader_module->function_ = [shader_module->library_ newFunctionWithName:entry_point];
    assert(shader_module->function_ != nil);

    return shader_module;
}

std::shared_ptr<QueueMtl> DeviceMtl::wrap_queue(id<MTLCommandQueue> mtl_cmd_queue, bool has_resource_ownership) {
    return std::shared_ptr<QueueMtl>(new QueueMtl(mtl_device_, mtl_cmd_queue));
}

DeviceMtl::~DeviceMtl() {}

std::shared_ptr<TextureMtl> DeviceMtl::wrap_texture(id<MTLTexture> mtl_texture, bool has_resource_ownership) {
    TextureDescriptor descriptor;
    descriptor.size = {(int32_t)mtl_texture.width, (int32_t)mtl_texture.height};
    descriptor.format = from_mtl_pixel_format(mtl_texture.pixelFormat);

    auto texture = std::shared_ptr<TextureMtl>(new TextureMtl(descriptor, "wrapped texture"));
    texture->mtl_texture_ = mtl_texture;
    return texture;
}

std::shared_ptr<Fence> DeviceMtl::create_fence(const std::string &label) {
    return std::shared_ptr<Fence>(new Fence());
}

std::shared_ptr<Buffer> DeviceMtl::create_staging_buffer(size_t size) {
    BufferDescriptor desc;
    desc.type = BufferType::Storage;
    desc.size = size;
    desc.property = MemoryProperty::HostVisibleAndCoherent;
    return create_buffer(desc, "Metal Staging Buffer");
}

void *DeviceMtl::map_staging(const StagingAllocation &allocation) {
    auto buffer_mtl = (BufferMtl *)allocation.buffer.get();
    return (uint8_t *)buffer_mtl->contents() + allocation.offset;
}

void DeviceMtl::unmap_staging(const StagingAllocation &allocation) {
}

std::shared_ptr<CommandEncoder> DeviceMtl::create_command_encoder(const std::string &label) {
    auto encoder = std::shared_ptr<CommandEncoderMtl>(new CommandEncoderMtl(mtl_device_, mtl_cmd_queue_));
    encoder->device_ = shared_from_this();
    return encoder;
}

std::shared_ptr<Framebuffer> DeviceMtl::create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                           const std::shared_ptr<Texture> &texture,
                                                           const std::string &label) {
    auto framebuffer = std::make_shared<FramebufferMtl>(texture, label);
    return framebuffer;
}

std::shared_ptr<ComputePipeline> DeviceMtl::create_compute_pipeline(
    const std::shared_ptr<ShaderModule> &shader_module,
    const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout,
    const std::string &label) {
    auto shader_module_mtl = (ShaderModuleMtl *)shader_module.get();

    MTLComputePipelineDescriptor *pipeline_desc = [MTLComputePipelineDescriptor new];
    pipeline_desc.label = [NSString stringWithUTF8String:label.c_str()];
    pipeline_desc.computeFunction = shader_module_mtl->function();

    id<MTLLibrary> defaultLibrary = [mtl_device_ newDefaultLibrary];

    // Entry function in the compute shader.
    id<MTLFunction> kernelFunction = shader_module_mtl->function();

    NSError *err = nil;
    auto pipeline_state = [mtl_device_ newComputePipelineStateWithFunction:kernelFunction error:&err];

    if (pipeline_state == nil) {
        NSLog(@"Failed to create compute pipeline state: %@", [err localizedDescription]);
        return nullptr;
    }

    auto pipeline_mtl = std::shared_ptr<ComputePipelineMtl>(new ComputePipelineMtl());
    pipeline_mtl->pipeline_state_ = pipeline_state;

    return pipeline_mtl;
}

size_t DeviceMtl::get_aligned_uniform_size(size_t original_size) {
    // Metal uniform buffer alignment is usually 256 bytes for setVertexBuffer:offset:atIndex:
    // and similar methods on some hardware, but 16 or 4 bytes might work for constants.
    // However, 256 is the safest minimum alignment for buffer offsets.
    const size_t alignment = 256;
    return (original_size + alignment - 1) & ~(alignment - 1);
}

} // namespace Pathfinder
