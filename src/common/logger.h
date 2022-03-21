//
// Created by floppyhammer on 2021/12/23.
//

#ifndef PATHFINDER_LOGGER_H
#define PATHFINDER_LOGGER_H

#include <memory>
#include <iostream>

#define PATHFINDER_LOG_TAG "Pathfinder"
#ifdef __ANDROID__
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, PATHFINDER_LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , PATHFINDER_LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , PATHFINDER_LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , PATHFINDER_LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , PATHFINDER_LOG_TAG, __VA_ARGS__)
#else

#include <cstdio>

#define LOGV(...) printf("<%s>", PATHFINDER_LOG_TAG); printf(__VA_ARGS__); printf("\n")
#define LOGD(...) printf("<%s>", PATHFINDER_LOG_TAG); printf(__VA_ARGS__); printf("\n")
#define LOGI(...) printf("<%s>", PATHFINDER_LOG_TAG); printf(__VA_ARGS__); printf("\n")
#define LOGW(...) printf("<%s>", PATHFINDER_LOG_TAG); printf(__VA_ARGS__); printf("\n")
#define LOGE(...) printf("<%s>", PATHFINDER_LOG_TAG); printf(__VA_ARGS__); printf("\n")
#endif

namespace Pathfinder {
    class Logger {
    public:
        static Logger &get_singleton() {
            static Logger singleton;
            return singleton;
        }

        enum Level {
            VERBOSE = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
        } level = Level::INFO;

        static void set_level(Level p_level) {
            get_singleton().level = p_level;
        }

        static void verbose(const std::string &label, const std::string &module = "") {
            if (get_singleton().level <= Level::VERBOSE) {
                LOGV("[VERBOSE][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
            }
        }

        static void debug(const std::string &label, const std::string &module = "") {
            if (get_singleton().level <= Level::DEBUG) {
                LOGD("[DEBUG][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
            }
        }

        static void info(const std::string &label, const std::string &module = "") {
            if (get_singleton().level <= Level::INFO) {
                LOGI("[INFO][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
            }
        }

        static void warn(const std::string &label, const std::string &module = "") {
            if (get_singleton().level <= Level::WARN) {
                LOGW("[WARN][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
            }
        }

        static void error(const std::string &label, const std::string &module = "") {
            if (get_singleton().level <= Level::ERROR) {
                LOGE("[ERROR][%s] %s", (module.empty() ? "default" : module).c_str(), label.c_str());
            }
        }

    private:
        // So we can't be instantiated by outsiders.
        Logger() = default;

    public:
        Logger(Logger const &) = delete;

        void operator=(Logger const &) = delete;
    };
}

#endif //PATHFINDER_LOGGER_H
