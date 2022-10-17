#include "renderer.h"

#include <umHalf.h>

#include <array>

#include "paint/palette.h"
#include "../common/io.h"
#include "../common/math/basic.h"
#include "../gpu/command_buffer.h"
#include "../gpu/platform.h"
#include "../shaders/generated/area_lut_png.h"

namespace Pathfinder {

Renderer::Renderer(const std::shared_ptr<Driver> &p_driver) {
    driver = p_driver;
}

void Renderer::set_up() {
    auto cmd_buffer = driver->create_command_buffer(true);

    // Uniform buffer.
    {
        fixed_sizes_ub =
            driver->create_buffer(BufferType::Uniform, 8 * sizeof(float), MemoryProperty::HOST_VISIBLE_AND_COHERENT);

        // Upload data to the uniform buffer with fixed data.
        std::array<float, 6> fixed_sizes_ubo_data = {MASK_FRAMEBUFFER_WIDTH,
                                                     MASK_FRAMEBUFFER_HEIGHT,
                                                     TILE_WIDTH,
                                                     TILE_HEIGHT,
                                                     TEXTURE_METADATA_TEXTURE_WIDTH,
                                                     TEXTURE_METADATA_TEXTURE_HEIGHT};

        cmd_buffer->upload_to_buffer(fixed_sizes_ub, 0, 6 * sizeof(float), fixed_sizes_ubo_data.data());
    }

    auto image_data = ImageData::from_memory({std::begin(area_lut_png), std::end(area_lut_png)}, false);

    area_lut_texture = driver->create_texture(image_data->width, image_data->height, TextureFormat::RGBA8_UNORM);

    cmd_buffer->upload_to_texture(area_lut_texture, {}, image_data->data, TextureLayout::SHADER_READ_ONLY);

    cmd_buffer->submit(driver);
}

} // namespace Pathfinder
