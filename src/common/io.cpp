//
// Created by floppyhammer on 2022/1/7.
//

#include "io.h"

#include "logger.h"

#include <stdexcept>

namespace Pathfinder {
    std::string load_file_as_string(const char *file_path) {
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

        return output;
    }

    std::vector<char> load_file_as_bytes(const char *file_path) {
        std::ifstream input(file_path, std::ios::binary);

        std::vector<char> bytes((std::istreambuf_iterator<char>(input)),
                                (std::istreambuf_iterator<char>()));

        input.close();

        return bytes;
    }
}
