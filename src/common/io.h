//
// Created by floppyhammer on 2022/1/7.
//

#ifndef PATHFINDER_IO_H
#define PATHFINDER_IO_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

namespace Pathfinder {
    std::string load_file_as_string(const char *file_path);

    std::vector<char> load_file_as_bytes(const char *file_path);

    struct ImageData {
        ~ImageData();

        static std::shared_ptr<ImageData> from_memory(const std::vector<unsigned char> &bytes, bool flip_y);
        static std::shared_ptr<ImageData> from_file(const char *file_path, bool flip_y);

        int32_t width, height, channel_count;
        unsigned char *data;
    };
}

#endif //PATHFINDER_IO_H
