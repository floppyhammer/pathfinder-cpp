//
// Created by chy on 2022/1/7.
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
}

#endif //PATHFINDER_IO_H
