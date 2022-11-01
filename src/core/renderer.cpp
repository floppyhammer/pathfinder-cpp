#include "renderer.h"

#include <umHalf.h>

#include <array>

#include "../common/io.h"
#include "../common/math/basic.h"
#include "../gpu/command_buffer.h"
#include "../gpu/platform.h"
#include "../shaders/generated/area_lut_png.h"
#include "paint/palette.h"

namespace Pathfinder {

Renderer::Renderer(const std::shared_ptr<Driver> &_driver) {
    driver = _driver;

    auto cmd_buffer = driver->create_command_buffer(true);

    // Uniform buffer for some constants.
    constants_ub =
        driver->create_buffer(BufferType::Uniform, 8 * sizeof(float), MemoryProperty::HostVisibleAndCoherent);

    std::array<float, 6> constants = {MASK_FRAMEBUFFER_WIDTH,
                                      MASK_FRAMEBUFFER_HEIGHT,
                                      TILE_WIDTH,
                                      TILE_HEIGHT,
                                      TEXTURE_METADATA_TEXTURE_WIDTH,
                                      TEXTURE_METADATA_TEXTURE_HEIGHT};

    cmd_buffer->upload_to_buffer(constants_ub, 0, 6 * sizeof(float), constants.data());

    // Area-Lut texture.
    auto image_buffer = ImageBuffer::from_memory({std::begin(area_lut_png), std::end(area_lut_png)}, false);

    area_lut_texture = driver->create_texture(image_buffer->width, image_buffer->height, TextureFormat::Rgba8Unorm);

    cmd_buffer->upload_to_texture(area_lut_texture, {}, image_buffer->data, TextureLayout::ShaderReadOnly);

    cmd_buffer->submit(driver);
}

} // namespace Pathfinder
