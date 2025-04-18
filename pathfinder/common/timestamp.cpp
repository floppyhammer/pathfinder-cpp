#include "timestamp.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "global_macros.h"
#include "logger.h"

namespace Pathfinder {

Timestamp::Timestamp(const std::string &logger_tag) {
    logger_tag_ = logger_tag;
    start_time = std::chrono::steady_clock::now();
}

void Timestamp::record(const std::string &label) {
    if (!enabled_) {
        return;
    }

    std::chrono::time_point<std::chrono::steady_clock> current_time = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed_time = current_time - start_time;

    auto elapsed_time_in_ms = round(elapsed_time.count() * 100000.0f) * 0.01f;

    records.push_back(elapsed_time_in_ms);
    labels.push_back(label);

    start_time = std::chrono::steady_clock::now();
}

void Timestamp::reset() {
    records.clear();
    labels.clear();
    start_time = std::chrono::steady_clock::now();
}

void Timestamp::print() const {
    if (!enabled_) {
        return;
    }

    for (int i = 0; i < records.size(); i++) {
        std::ostringstream string_stream;
        string_stream << i << " " << labels[i] << " " << records[i] << " ms";
        Logger::info(string_stream.str(), logger_tag_);
    }
}

void Timestamp::set_enabled(bool enabled) {
    enabled_ = enabled;
}

} // namespace Pathfinder
