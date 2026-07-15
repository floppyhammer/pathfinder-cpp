#include "device.h"

namespace Pathfinder {

StagingAllocation Device::allocate_staging(size_t size) {
    if (size > STAGING_BLOCK_SIZE) {
        auto buffer = create_staging_buffer(size);

        StagingAllocation alloc;
        alloc.buffer = buffer;
        alloc.offset = 0;
        alloc.data_size = size;
        alloc.mapped_ptr = nullptr;
        return alloc;
    }

    auto &bucket = staging_buckets_[current_frame_index_ % frames_in_flight_];

    for (auto &block : bucket.blocks) {
        size_t aligned_offset = (block.used_size + 15) & ~15;

        if (aligned_offset + size <= STAGING_BLOCK_SIZE) {
            StagingAllocation alloc;
            alloc.buffer = block.buffer;
            alloc.offset = aligned_offset;
            alloc.data_size = size;
            alloc.mapped_ptr = nullptr;

            block.used_size = aligned_offset + size;
            return alloc;
        }
    }

    auto buffer = create_staging_buffer(STAGING_BLOCK_SIZE);

    StagingBlock new_block;
    new_block.buffer = buffer;
    new_block.used_size = size;
    bucket.blocks.push_back(new_block);

    StagingAllocation alloc;
    alloc.buffer = buffer;
    alloc.offset = 0;
    alloc.data_size = size;
    alloc.mapped_ptr = nullptr;

    return alloc;
}

void Device::reset_staging() {
    for (auto &bucket : staging_buckets_) {
        for (auto &block : bucket.blocks) {
            block.used_size = 0;
        }
    }
}

} // namespace Pathfinder