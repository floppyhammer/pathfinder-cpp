#ifndef PATHFINDER_LOGGER_H
#define PATHFINDER_LOGGER_H

#include <iostream>
#include <memory>

#define PATHFINDER_LOG_TAG "Pathfinder"
#ifdef __ANDROID__
    #include <android/log.h>
    #define PATHFINDER_LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, PATHFINDER_LOG_TAG, __VA_ARGS__)
    #define PATHFINDER_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, PATHFINDER_LOG_TAG, __VA_ARGS__)
    #define PATHFINDER_LOGI(...) __android_log_print(ANDROID_LOG_INFO, PATHFINDER_LOG_TAG, __VA_ARGS__)
    #define PATHFINDER_LOGW(...) __android_log_print(ANDROID_LOG_WARN, PATHFINDER_LOG_TAG, __VA_ARGS__)
    #define PATHFINDER_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, PATHFINDER_LOG_TAG, __VA_ARGS__)
#else

    #include <cstdio>

    #define PATHFINDER_LOGV(...)            \
        printf("<%s>", PATHFINDER_LOG_TAG); \
        printf(__VA_ARGS__);                \
        printf("\n")
    #define PATHFINDER_LOGD(...)            \
        printf("<%s>", PATHFINDER_LOG_TAG); \
        printf(__VA_ARGS__);                \
        printf("\n")
    #define PATHFINDER_LOGI(...)            \
        printf("<%s>", PATHFINDER_LOG_TAG); \
        printf(__VA_ARGS__);                \
        printf("\n")
    #define PATHFINDER_LOGW(...)            \
        printf("<%s>", PATHFINDER_LOG_TAG); \
        printf(__VA_ARGS__);                \
        printf("\n")
    #define PATHFINDER_LOGE(...)            \
        printf("<%s>", PATHFINDER_LOG_TAG); \
        printf(__VA_ARGS__);                \
        printf("\n")
#endif

namespace Pathfinder {
class Logger {
public:
    static Logger *get_singleton() {
        static Logger singleton;
        return &singleton;
    }

    enum class Level {
        VERBOSE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
    } level = Level::INFO;

    static void set_level(Level p_level) {
        get_singleton()->level = p_level;
    }

    static void verbose(const std::string &label, const std::string &module = "") {
        if (get_singleton()->level <= Level::VERBOSE) {
            PATHFINDER_LOGV("[VERBOSE][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
        }
    }

    static void debug(const std::string &label, const std::string &module = "") {
        if (get_singleton()->level <= Level::DEBUG) {
            PATHFINDER_LOGD("[DEBUG][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
        }
    }

    static void info(const std::string &label, const std::string &module = "") {
        if (get_singleton()->level <= Level::INFO) {
            PATHFINDER_LOGI("[INFO][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
        }
    }

    static void warn(const std::string &label, const std::string &module = "") {
        if (get_singleton()->level <= Level::WARN) {
            PATHFINDER_LOGW("[WARN][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
        }
    }

    static void error(const std::string &label, const std::string &module = "") {
        if (get_singleton()->level <= Level::ERROR) {
            PATHFINDER_LOGE("[ERROR][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
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
