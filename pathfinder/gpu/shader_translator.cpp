#include "shader_translator.h"

#ifdef PATHFINDER_USE_GLSLANG

    #include <glslang/Public/ResourceLimits.h>
    #include <glslang/Public/ShaderLang.h>
    #include <glslang/SPIRV/GlslangToSpv.h>

    #include <cmath>
    #include <iostream>
    #include <ostream>
    #include <spirv-cross/spirv_glsl.hpp>
    #include <spirv-cross/spirv_msl.hpp>

    #include "../common/logger.h"

namespace Pathfinder {

ShaderTranslator::ShaderTranslator(ShaderStage stage) {
    stage_ = stage;
    glslang::InitializeProcess();
}

ShaderTranslator::~ShaderTranslator() {
    glslang::FinalizeProcess();
}

std::shared_ptr<ShaderCode> glsl_to_spv(const ShaderCode *glsl_code) {
    EShLanguage shader_stage;

    switch (glsl_code->stage) {
        case ShaderStage::Vertex:
            shader_stage = EShLangVertex;
            break;
        case ShaderStage::Fragment:
            shader_stage = EShLangFragment;
            break;
        case ShaderStage::Compute:
            shader_stage = EShLangCompute;
            break;
        case ShaderStage::Geometry:
            shader_stage = EShLangGeometry;
            break;
        default:
            abort();
    }

    const char *shader_code_c_str = glsl_code->code.c_str();

    glslang::TShader shader(shader_stage);
    shader.setStrings(&shader_code_c_str, 1);

    // Set environment for Vulkan 1.0
    shader.setEnvInput(glslang::EShSource::EShSourceGlsl,
                       shader_stage,
                       glslang::EShClient::EShClientVulkan,
                       glslang::EShTargetClientVersion::EShTargetVulkan_1_0);
    shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0);

    // Default resources, you might need to customize this for your specific needs
    const TBuiltInResource *default_resources = GetDefaultResources();

    auto messages = (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules);

    // 450 stands for GLSL version 4.50.
    if (!shader.parse(default_resources, 450, true, messages)) {
        std::cerr << "GLSL Parse Failure:\n" << shader.getInfoLog() << std::endl;
        return {};
    }

    // Link the shader.
    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages)) {
        std::cerr << "GLSL Link Failure:\n" << program.getInfoLog() << std::endl;
        return {};
    }

    std::vector<uint32_t> spv_binary;
    glslang::GlslangToSpv(*program.getIntermediate(shader_stage), spv_binary);

    auto spv_code = std::make_shared<ShaderCode>();
    spv_code->entry_point = glsl_code->entry_point;
    spv_code->stage = glsl_code->stage;
    spv_code->code = std::string(reinterpret_cast<char *>(&spv_binary[0]), spv_binary.size() * sizeof(uint32_t));

    return spv_code;
}

std::shared_ptr<ShaderCode> spv_to_glsl(const ShaderCode *spv_code, bool is_es, bool need_framebuffer_fetch) {
    spirv_cross::CompilerGLSL::Options options;
    if (is_es) {
        options.es = true;
        options.version = 300;
        if (spv_code->stage == ShaderStage::Compute) {
            options.version = 310;
        }
    } else {
        options.es = false;
        options.version = 450;
    }
    options.vertex.flip_vert_y = false;
    options.vertex.fixup_clipspace = true;

    spirv_cross::CompilerGLSL glsl(reinterpret_cast<const uint32_t *>(spv_code->code.data()),
                                   spv_code->code.size() / 4);
    glsl.set_common_options(options);

    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    auto glsl_code = std::make_shared<ShaderCode>();
    glsl_code->stage = spv_code->stage;
    glsl_code->entry_point = spv_code->entry_point;

    if (is_es && ShaderStage::Fragment == spv_code->stage && need_framebuffer_fetch) {
        glsl.remap_ext_framebuffer_fetch(0, 0, true);
        glsl.remap_ext_framebuffer_fetch(1, 1, true);
        glsl.remap_ext_framebuffer_fetch(2, 2, true);
        glsl.remap_ext_framebuffer_fetch(3, 3, true);
    }

    glsl_code->code = glsl.compile();

    // Add custom binding info.
    {
        std::string binding_info = "//BINDING_START\n";

        // 获取经过 SPIRV-Cross 翻译后的最终资源信息
        // 关键：直接从 Compiler 对象中获取，不要在 compile() 之前 unset 任何东西
        auto active_resources = glsl.get_shader_resources();

        // Get sampler info.
        for (auto &resource : active_resources.sampled_images) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            // 计算扁平化后的绑定点 (Set * 16 + Binding)
            unsigned final_binding = set * 16 + binding;

            binding_info += "//TEX:" + resource.name + ":" + std::to_string(final_binding) + "\n";
        }

        for (auto &resource : active_resources.uniform_buffers) {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            unsigned final_binding = set * 16 + binding;

            binding_info += "//UBO:" + resource.name + ":" + std::to_string(final_binding) + "\n";
        }

        binding_info += "//BINDING_END\n";
        glsl_code->code = binding_info + glsl_code->code;
    }

    // Get shader resources.
    auto resource = glsl.get_shader_resources();

    // Update glsl resource binding map for GLES 3.0.

    // Update texture binding map.
    for (const auto &r : resource.sampled_images) {
        uint32_t binding = glsl.get_decoration(r.id, spv::DecorationBinding);
        glsl_code->texture_binding_map.emplace_back(binding, r.name);
    }

    // Update uniform binding map.
    std::vector<UniformBufferInfo> ubo_infos_;

    for (const auto &r : resource.uniform_buffers) {
        uint32_t binding = glsl.get_decoration(r.id, spv::DecorationBinding);
        glsl_code->uniform_buffer_binding_map.emplace_back(binding, r.name);

        // Update uniform buffer info.
        ubo_infos_.emplace_back();
        auto &ubo_info = ubo_infos_.back();
        const auto &ub_type = glsl.get_type(r.type_id);
        ubo_info.name = r.name;
        ubo_info.binding_point = binding;
        ubo_info.size = std::ceil(glsl.get_declared_struct_size(ub_type) / 16.0f) * 16;
        ubo_info.elements.resize(ub_type.member_types.size());
        ubo_info.stage = static_cast<ShaderStage>(spv_code->stage);
        for (auto i = 0; i < ubo_info.elements.size(); ++i) {
            ubo_info.elements[i].name = glsl.get_member_name(ub_type.self, i);
            ubo_info.elements[i].offset = glsl.type_struct_member_offset(ub_type, i);
            ubo_info.elements[i].size = glsl.get_declared_struct_member_size(ub_type, i);
        }
    }

    return glsl_code;
}

std::shared_ptr<ShaderCode> spv_to_msl(const ShaderCode *spv_code, bool need_framebuffer_fetch) {
    spirv_cross::CompilerMSL msl(reinterpret_cast<const uint32_t *>(spv_code->code.data()), spv_code->code.size() / 4);
    spirv_cross::ShaderResources resources = msl.get_shader_resources();

    // Metal 专有的绑定偏移逻辑
    // 预留前 8 个索引给 Vertex Buffers ([[buffer(0)]] 到 [[buffer(7)]])
    const unsigned METAL_BUFFER_OFFSET = 8;

    for (const auto &resource : resources.uniform_buffers) {
        const unsigned set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const unsigned binding = msl.get_decoration(resource.id, spv::DecorationBinding);

        msl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
        // MSL 中 UBO 和 Vertex Buffer 共享 buffer 索引，所以必须偏移
        msl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding + METAL_BUFFER_OFFSET);
    }

    for (const auto &resource : resources.sampled_images) {
        const unsigned set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const unsigned binding = msl.get_decoration(resource.id, spv::DecorationBinding);

        msl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
        // 采样器通常使用 [[texture(N)]] 和 [[sampler(N)]]，与 buffer 空间分开，
        // 但为了统一管理，我们也按照同样的规则扁平化
        msl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    }

    spirv_cross::CompilerMSL::Options options;
    options.enable_decoration_binding = true;
    options.msl_version = 120;
    if (need_framebuffer_fetch && spv_code->stage == ShaderStage::Fragment) {
        options.use_framebuffer_fetch_subpasses = true;
    }
    options.platform = spirv_cross::CompilerMSL::Options::Platform::iOS;
    msl.set_msl_options(options);

    auto msl_code = std::make_shared<ShaderCode>();
    msl_code->stage = spv_code->stage;
    msl_code->entry_point = spv_code->entry_point;
    msl_code->code = msl.compile();

    return msl_code;
}

void ShaderTranslator::set_shader_code(const ShaderCodeKey &shader_key, const std::shared_ptr<ShaderCode> &code) {
    shader_->update_shader_code(shader_key, code);
}

void ShaderTranslator::compile_from_glsl(const std::string &entry_point,
                                         const std::string &shader_code,
                                         bool need_framebuffer_fetch) {
    // Reset shader since prepare() only overwrites nonexistent shader keys.
    shader_ = std::make_shared<Shader>();

    auto glsl_code = std::make_shared<ShaderCode>();
    glsl_code->stage = stage_;
    glsl_code->entry_point = entry_point;
    glsl_code->code = shader_code;

    set_shader_code({ShaderSourceType::GLSL, 4, 5}, glsl_code);

    prepare(need_framebuffer_fetch);
}

std::shared_ptr<Shader> ShaderTranslator::get_shader() const {
    return shader_;
}

bool ShaderTranslator::prepare(bool need_framebuffer_fetch) {
    auto spv_code_ptr = shader_->get_shader_code({ShaderSourceType::SPIRV, 1, 1});
    auto glsl_code_ptr = shader_->get_shader_code({ShaderSourceType::GLSL, 4, 5});
    auto es_code_ptr = shader_->get_shader_code({ShaderSourceType::GLSLES, 3, 0});
    auto msl_code_ptr = shader_->get_shader_code({ShaderSourceType::MSL, 1, 2});

    // Generate spv and glsl code.
    if (spv_code_ptr == nullptr) {
        // Compile glsl and convert it to spv.
        if (glsl_code_ptr == nullptr) {
            Logger::error("No glsl 4.5 shader code found!");
        }

        // Generate Spv code.
        auto spv_code = glsl_to_spv(glsl_code_ptr.get());

        shader_->update_shader_code({ShaderSourceType::SPIRV, 1, 1}, spv_code);

        spv_code_ptr = shader_->get_shader_code({ShaderSourceType::SPIRV, 1, 1});

        // Modify the original GLSL shader code.
        auto new_glsl_code = spv_to_glsl(spv_code_ptr.get(), false, need_framebuffer_fetch);
        shader_->update_shader_code({ShaderSourceType::GLSL, 4, 5}, new_glsl_code);
    }

    // Generate es code.
    if (es_code_ptr == nullptr) {
        auto minor_version = stage_ == ShaderStage::Compute ? 1u : 0u;
        auto es_code = spv_to_glsl(spv_code_ptr.get(), true, need_framebuffer_fetch);
        shader_->update_shader_code({ShaderSourceType::GLSLES, 3, uint8_t(minor_version)}, es_code);
    }

    // Generate msl code.
    if (msl_code_ptr == nullptr) {
        auto msl_code = spv_to_msl(spv_code_ptr.get(), need_framebuffer_fetch);
        shader_->update_shader_code({ShaderSourceType::MSL, 1, 2}, msl_code);
    }

    return true;
}

} // namespace Pathfinder

#endif
