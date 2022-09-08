#ifndef PATHFINDER_IO_H
#define PATHFINDER_IO_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

namespace Pathfinder {
    std::string load_file_as_string(const std::string &file_path);

    std::vector<char> load_file_as_bytes(const std::string &file_path);

    struct ImageData {
        ~ImageData();

        static std::shared_ptr<ImageData> from_memory(const std::vector<char> &bytes, bool flip_y);
        static std::shared_ptr<ImageData> from_file(const char *file_path, bool flip_y);

        int32_t width, height, channel_count;
        unsigned char *data;
    };
}

#endif //PATHFINDER_IO_H
