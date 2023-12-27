#include "io.h"

#include "logger.h"
#include "timestamp.h"

#ifndef STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>

#include <cerrno>
#include <stdexcept>

namespace Pathfinder {

#ifndef __ANDROID__
std::string load_file_as_string(const std::string &file_path) {
    Timestamp timer;

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

    timer.record("Loading file as string: " + file_path);
    timer.print();

    return std::move(output);
}

std::vector<char> load_file_as_bytes(const std::string &file_path) {
    Timestamp timer;

    FILE *file = fopen(file_path.c_str(), "rb");
    if (!file) {
        Logger::error("Failed to load file: " + file_path);
        return {};
    }

    fseek(file, 0, SEEK_END);
    const long length = ftell(file);
    std::vector<char> bytes(length);
    fseek(file, 0, SEEK_SET);

    const size_t ret = fread(bytes.data(), 1, length, file);
    if (ret != length) {
        Logger::error("Error reading file: " + file_path);
    }

    fclose(file);

    timer.record("Loading file as bytes: " + file_path);
    timer.print();

    return std::move(bytes);
}
#endif

std::shared_ptr<ImageBuffer> ImageBuffer::from_memory(const std::vector<char> &bytes, bool flip_y) {
    stbi_set_flip_vertically_on_load(flip_y);

    // Always load images as RGBA pixels.
    int32_t width, height, channels;
    unsigned char *img_data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(bytes.data()),
                                                    bytes.size() * sizeof(char),
                                                    &width,
                                                    &height,
                                                    &channels,
                                                    STBI_rgb_alpha);

    if (channels != 4) {
        Logger::info("Converted non-RGBA pixels to RGBA ones", "ImageBuffer");
    }

    // Generate a texture using the previously loaded image data.
    if (!img_data) {
        Logger::error("Failed to load image from memory!", "ImageBuffer");
        return nullptr;
    }

    auto image_buffer = std::make_shared<ImageBuffer>();
    image_buffer->size = {width, height};
    image_buffer->channel_count = 4;
    image_buffer->data = img_data;

    return image_buffer;
}

ImageBuffer::~ImageBuffer() {
    if (data) {
        stbi_image_free(data);
    }
}

std::vector<ColorU> ImageBuffer::to_rgba_pixels() const {
    std::vector<ColorU> pixels(size.area());

    memcpy(pixels.data(), data, size.area() * channel_count);

    return pixels;
}

Vec2I ImageBuffer::get_size() const {
    return size;
}

unsigned char *ImageBuffer::get_data() const {
    if (data == nullptr) {
        Logger::error("Try to get data from an invalid image buffer!", "ImageBuffer");
    }
    return data;
}

} // namespace Pathfinder
