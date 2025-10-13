#ifndef PATHFINDER_IO_H
#define PATHFINDER_IO_H

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "color.h"
#include "math/vec2.h"

namespace Pathfinder {

#ifndef __ANDROID__
std::vector<char> load_file_as_bytes(const std::string &file_path);

std::string load_file_as_string(const std::string &file_path);
#else

std::vector<char> load_asset(void *asset_manager, const std::string &filename);
#endif

class ImageBuffer {
public:
    ~ImageBuffer();

    static std::shared_ptr<ImageBuffer> from_memory(const std::vector<char> &bytes, bool flip_y);

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
