#pragma once

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#ifdef __ANDROID__
    #include <android/log.h>
#endif

#define PATHFINDER_DEFAULT_LOG_TAG "PF"

namespace Pathfinder {

class Logger {
public:
    static Logger *get_singleton() {
        static Logger singleton;
        return &singleton;
    }

    /// Effective level = MAX(global level, module level)
    enum class Level {
        Verbose = 0,
        Debug,
        Info,
        Warn,
        Error,
        Silence,
    };

    static void set_global_level(const Level level) {
        get_singleton()->global_level_.store(level, std::memory_order_relaxed);
    }

    static void set_module_level(const std::string &module, const Level level) {
        if (module.empty()) {
            return;
        }
        auto *s = get_singleton();
        std::unique_lock<std::shared_timed_mutex> lock(s->level_mutex_);
        s->module_levels[module] = level;
    }

    static Level get_effective_level(const std::string &module) {
        auto *s = get_singleton();
        const auto global_level = s->global_level_.load(std::memory_order_relaxed);

        if (module.empty()) {
            return global_level;
        }

        std::shared_lock<std::shared_timed_mutex> lock(s->level_mutex_);
        auto it = s->module_levels.find(module);
        if (it == s->module_levels.end()) {
            return global_level;
        }

        const auto module_level = it->second;

        return global_level > module_level ? global_level : module_level;
    }

    static void verbose(const std::string &label, const std::string &module = PATHFINDER_DEFAULT_LOG_TAG) {
        log(Level::Verbose, label, module);
    }

    static void debug(const std::string &label, const std::string &module = PATHFINDER_DEFAULT_LOG_TAG) {
        log(Level::Debug, label, module);
    }

    static void info(const std::string &label, const std::string &module = PATHFINDER_DEFAULT_LOG_TAG) {
        log(Level::Info, label, module);
    }

    static void warn(const std::string &label, const std::string &module = PATHFINDER_DEFAULT_LOG_TAG) {
        log(Level::Warn, label, module);
    }

    static void error(const std::string &label, const std::string &module = PATHFINDER_DEFAULT_LOG_TAG) {
        log(Level::Error, label, module);
    }

private:
    // So it can't be instantiated by outsiders.
    Logger() : global_level_(Level::Info) {}

    static void log(Level level, const std::string &label, const std::string &module) {
        auto *s = get_singleton();

        // Fast path: Atomic check against global level.
        // If the message level is lower than the global floor, we can skip everything immediately.
        if (level < s->global_level_.load(std::memory_order_relaxed)) {
            return;
        }

        // Slow path: Check if the specific module has a stricter level.
        if (get_effective_level(module) <= level) {
            std::lock_guard<std::mutex> lock(s->output_mutex_);

#ifdef __ANDROID__
            int android_level = ANDROID_LOG_INFO;
            switch (level) {
                case Level::Verbose:
                    android_level = ANDROID_LOG_VERBOSE;
                    break;
                case Level::Debug:
                    android_level = ANDROID_LOG_DEBUG;
                    break;
                case Level::Info:
                    android_level = ANDROID_LOG_INFO;
                    break;
                case Level::Warn:
                    android_level = ANDROID_LOG_WARN;
                    break;
                case Level::Error:
                    android_level = ANDROID_LOG_ERROR;
                    break;
                default:
                    break;
            }
            __android_log_write(android_level, module.c_str(), label.c_str());
#else
            const char *level_str = "[INFO]";
            std::ostream *os = &std::cout;

            switch (level) {
                case Level::Verbose:
                    level_str = "[VERBOSE]";
                    break;
                case Level::Debug:
                    level_str = "[DEBUG]";
                    break;
                case Level::Info:
                    level_str = "[INFO]";
                    break;
                case Level::Warn:
                    level_str = "[WARN]";
                    os = &std::cerr;
                    break;
                case Level::Error:
                    level_str = "[ERROR]";
                    os = &std::cerr;
                    break;
                default:
                    break;
            }

            *os << "<" << module << ">" << level_str << " " << label << std::endl;
#endif
        }
    }

    std::atomic<Level> global_level_;
    std::unordered_map<std::string, Level> module_levels;

    mutable std::shared_timed_mutex level_mutex_;
    mutable std::mutex output_mutex_;

public:
    Logger(Logger const &) = delete;

    void operator=(Logger const &) = delete;
};

} // namespace Pathfinder
