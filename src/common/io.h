#ifndef PATHFINDER_IO_H
#define PATHFINDER_IO_H

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "color.h"
#include "math/vec2.h"

namespace Pathfinder {

std::string load_file_as_string(const std::string &file_path);

/// Load a file in binary mode.
std::vector<char> load_file_as_bytes(const std::string &file_path);

class ImageBuffer {
public:
    ~ImageBuffer();

    static std::shared_ptr<ImageBuffer> from_memory(const std::vector<char> &bytes, bool flip_y);

    static std::shared_ptr<ImageBuffer> from_file(const std::string &file_path, bool flip_y);

    std::vector<ColorU> to_rgba_pixels() const;

    Vec2I get_size() const;

    unsigned char *get_data() const;

private:
    Vec2I size;

    int32_t channel_count{};

    unsigned char *data{};
};

} // namespace Pathfinder

#endif // PATHFINDER_IO_H
