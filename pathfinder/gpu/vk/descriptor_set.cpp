#include "descriptor_set.h"

#include <cassert>

#include "buffer.h"
#include "texture.h"

namespace Pathfinder {

void DescriptorSetVk::update_vk_descriptor_set(VkDevice vk_device, VkDescriptorSetLayout vk_descriptor_set_layout) {
    vk_device_ = vk_device;

    if (!dirty) {
        return;
    }

    // Create descriptor pool and allocate descriptor sets.
    if (!descriptor_set_allocated_) {
        // Get pool sizes.
        std::vector<VkDescriptorPoolSize> pool_sizes{};

        for (auto &d : descriptors) {
            VkDescriptorPoolSize pool_size{};
            pool_size.descriptorCount = 1;

            switch (d.second.type) {
                case DescriptorType::UniformBuffer: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                } break;
                case DescriptorType::Sampler: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                } break;
                case DescriptorType::StorageBuffer: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                } break;
                case DescriptorType::Image: {
                    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                } break;
                default: {
                    abort();
                }
            }

            pool_sizes.push_back(pool_size);
        }

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = 1;

        VK_CHECK_RESULT(vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &vk_descriptor_pool_))

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = vk_descriptor_pool_;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &vk_descriptor_set_layout;

        VK_CHECK_RESULT(vkAllocateDescriptorSets(vk_device_, &alloc_info, &vk_descriptor_set_))

        descriptor_set_allocated_ = true;
    }

    // These have to be here to prevent them from going out of scope
    // before being used in vkUpdateDescriptorSets().
    std::vector<VkWriteDescriptorSet> descriptor_writes;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorImageInfo> image_infos;

    // A vector may reach its current capacity and reallocate its internal storage to a new memory location.
    // This is to make sure that doesn't happen.
    buffer_infos.reserve(descriptors.size());
    image_infos.reserve(descriptors.size());
    descriptor_writes.reserve(descriptors.size());

    for (auto &pair : descriptors) {
        auto &descriptor = pair.second;

        // Must initialize it with {}.
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = vk_descriptor_set_;
        write.dstBinding = descriptor.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        switch (descriptor.type) {
            case DescriptorType::UniformBuffer:
            case DescriptorType::StorageBuffer: {
                // It's possible to bind nothing at a slot in the layout.
                // However, you may see validation errors.
                if (descriptor.buffer == nullptr) {
                    continue;
                }

                auto buffer_vk = static_cast<BufferVk *>(descriptor.buffer.get());

                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = buffer_vk->get_vk_buffer();
                buffer_info.offset = descriptor.buffer_offset;
                buffer_info.range = descriptor.buffer_range;

                if (descriptor.type == DescriptorType::UniformBuffer) {
                    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                } else if (descriptor.type == DescriptorType::StorageBuffer) {
                    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }

                buffer_infos.push_back(buffer_info);
                write.pBufferInfo = &buffer_infos.back();
            } break;
            case DescriptorType::Sampler:
            case DescriptorType::Image: {
                // It's possible to bind nothing at a slot in the layout.
                // However, you may see validation errors.
                if (descriptor.texture == nullptr) {
                    continue;
                }

                auto texture_vk = static_cast<TextureVk *>(descriptor.texture.get());
                auto sampler_vk = static_cast<SamplerVk *>(descriptor.sampler.get());

                VkDescriptorImageInfo image_info{};
                image_info.imageView = texture_vk->get_image_view();

                if (descriptor.type == DescriptorType::Sampler) {
                    image_info.sampler = sampler_vk->get_sampler();
                    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                } else if (descriptor.type == DescriptorType::Image) {
                    image_info.sampler = VK_NULL_HANDLE;
                    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                }

                image_infos.push_back(image_info);
                write.pImageInfo = &image_infos.back();
            } break;
            default:
                abort();
        }

        descriptor_writes.push_back(write);
    }

    // To update multiple descriptor sets at once,
    // make sure the pBufferInfo or pImageInfo pointers are still valid at this point.
    vkUpdateDescriptorSets(vk_device_, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);

    dirty = false;
}

VkDescriptorSet &DescriptorSetVk::get_vk_descriptor_set() {
    return vk_descriptor_set_;
}

DescriptorSetVk::~DescriptorSetVk() {
    if (descriptor_set_allocated_) {
        // When we destroy the pool, the sets inside are destroyed as well.
        vkDestroyDescriptorPool(vk_device_, vk_descriptor_pool_, nullptr);
    }
}

} // namespace Pathfinder
