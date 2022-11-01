#include "io.h"

#include "logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

namespace Pathfinder {

std::string load_file_as_string(const std::string &file_path) {
    std::string output;
    std::ifstream file;

    // Ensure ifstream object can throw exceptions.
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // Open file.
        file.open(file_path);

        // Read file's buffer contents into stream.
        std::stringstream stream;
        stream << file.rdbuf();

        // Close file handler.
        file.close();

        // Convert stream into string.
        output = stream.str();
    } catch (std::ifstream::failure &e) {
        throw std::runtime_error(std::string("Failed to load string from disk: ") + std::string(file_path));
    }

    return std::move(output);
}

std::vector<char> load_file_as_bytes(const std::string &file_path) {
    std::ifstream input(file_path, std::ios::binary);

    std::vector<char> bytes((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));

    input.close();

    return std::move(bytes);
}

std::shared_ptr<ImageBuffer> ImageBuffer::from_memory(const std::vector<char> &bytes, bool flip_y) {
    stbi_set_flip_vertically_on_load(flip_y);

    int32_t img_width, img_height, img_channels;
    unsigned char *img_data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(bytes.data()),
                                                    bytes.size() * sizeof(char),
                                                    &img_width,
                                                    &img_height,
                                                    &img_channels,
                                                    0);

    // Generate a texture using the previously loaded image data.
    if (!img_data) {
        Logger::error("Failed to load image from memory!", "ImageData");
        throw std::runtime_error(std::string("Failed to load image from memory!"));
    }

    auto image_data = std::make_shared<ImageBuffer>();
    image_data->width = img_width;
    image_data->height = img_height;
    image_data->channel_count = img_channels;
    image_data->data = img_data;

    return image_data;
}

std::shared_ptr<ImageBuffer> ImageBuffer::from_file(const std::string &file_path, bool flip_y) {
    return ImageBuffer::from_memory(load_file_as_bytes(file_path), flip_y);
}

ImageBuffer::~ImageBuffer() {
    if (data) {
        stbi_image_free(data);
    }
}

std::vector<ColorU> ImageBuffer::to_rgba_pixels() const {
    if (channel_count != 4) {
        Logger::error("Cannot convert ImageData to RGBA pixels due to mismatched channel count!");
        return {};
    }

    std::vector<ColorU> pixels(width * height);

    memcpy(pixels.data(), data, width * height * channel_count);

    return pixels;
}

} // namespace Pathfinder
