#ifndef PATHFINDER_LOGGER_H
#define PATHFINDER_LOGGER_H

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#ifdef __ANDROID__
    #include <android/log.h>
#endif

#define PATHFINDER_DEFAULT_LOG_TAG "Pathfinder Default Logger"

namespace Pathfinder {
    class Logger {
    public:
        static Logger *get_singleton() {
            static Logger singleton;
            return &singleton;
        }

        enum class Level {
            Verbose = 0,
            Debug,
            Info,
            Warn,
            Error,
            Silence,
        } default_level_ = Level::Warn;

        std::unordered_map<std::string, Level> module_levels;

        static void set_default_level(Level level) {
            get_singleton()->default_level_ = level;
        }

        static void set_module_level(const std::string &module, Level level) {
            if (module.empty()) {
                return;
            }
            get_singleton()->module_levels[module] = level;
        }

        static Level get_module_level(const std::string &module) {
            if (module.empty()) {
                return get_singleton()->default_level_;
            }
            if (get_singleton()->module_levels.find(module) == get_singleton()->module_levels.end()) {
                return get_singleton()->default_level_;
            }
            return get_singleton()->module_levels.at(module);
        }

        static void verbose(const std::string &label, const std::string &module = "") {
            auto level = get_module_level(module);
            if (level <= Level::Verbose) {
                auto tag = (module.empty() ? PATHFINDER_DEFAULT_LOG_TAG : module).c_str();
#ifdef __ANDROID__
            __android_log_write(ANDROID_LOG_VERBOSE, tag, output.c_str());
#else
                std::cout << "<" << tag << ">[VERBOSE] " << label << std::endl;
#endif
            }
        }

        static void debug(const std::string &label, const std::string &module = "") {
            auto level = get_module_level(module);
            if (level <= Level::Debug) {
                auto tag = module.empty() ? PATHFINDER_DEFAULT_LOG_TAG : module.c_str();
#ifdef __ANDROID__
                __android_log_write(ANDROID_LOG_DEBUG, tag, label.c_str());
#else
                std::cout << "<" << tag << ">[DEBUG] " << label << std::endl;
#endif
            }
        }

        static void info(const std::string &label, const std::string &module = "") {
            auto level = get_module_level(module);
            if (level <= Level::Info) {
                auto tag = module.empty() ? PATHFINDER_DEFAULT_LOG_TAG : module.c_str();
#ifdef __ANDROID__
                __android_log_write(ANDROID_LOG_INFO, tag, label.c_str());
#else
                std::cout << "<" << tag << ">[INFO] " << label << std::endl;
#endif
            }
        }

        static void warn(const std::string &label, const std::string &module = "") {
            auto level = get_module_level(module);
            if (level <= Level::Warn) {
                auto tag = module.empty() ? PATHFINDER_DEFAULT_LOG_TAG : module.c_str();
#ifdef __ANDROID__
                __android_log_write(ANDROID_LOG_WARN, tag, label.c_str());
#else
                std::cout << "<" << tag << ">[WARN] " << label << std::endl;
#endif
            }
        }

        static void error(const std::string &label, const std::string &module = "") {
            auto level = get_module_level(module);
            if (level <= Level::Error) {
                auto tag = module.empty() ? PATHFINDER_DEFAULT_LOG_TAG : module.c_str();
#ifdef __ANDROID__
                __android_log_write(ANDROID_LOG_ERROR, tag, label.c_str());
#else
                std::cout << "<" << tag << ">[ERROR] " << label << std::endl;
#endif
            }
        }

    private:
        // So it can't be instantiated by outsiders.
        Logger() = default;

    public:
        Logger(Logger const &) = delete;

        void operator=(Logger const &) = delete;
    };
} // namespace Pathfinder

#endif // PATHFINDER_LOGGER_H
