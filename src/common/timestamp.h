#ifndef PATHFINDER_TIMESTAMP_H
#define PATHFINDER_TIMESTAMP_H

#include <chrono>
#include <string>
#include <vector>

namespace Pathfinder {

class Timestamp {
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;

    std::vector<double> records; // In ms.
    std::vector<std::string> labels;

    bool enabled = true;

public:
    Timestamp();

    void record(const std::string &p_label);

    void reset();

    void print();

    void set_enabled(bool p_enabled);
};

} // namespace Pathfinder

#endif // PATHFINDER_TIMESTAMP_H
