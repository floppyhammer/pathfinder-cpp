#ifndef PATHFINDER_TIMESTAMP_H
#define PATHFINDER_TIMESTAMP_H

#include <chrono>
#include <string>
#include <vector>

namespace Pathfinder {

class Timestamp {
    std::chrono::time_point<std::chrono::steady_clock> start_time;

    std::vector<double> records; // In ms.
    std::vector<std::string> labels;

    bool enabled_ = true;

public:
    Timestamp();

    void record(const std::string &label);

    void reset();

    void print() const;

    void set_enabled(bool enabled);
};

} // namespace Pathfinder

#endif // PATHFINDER_TIMESTAMP_H
