/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef NONOGRAMS_LOGGER_H_
#define NONOGRAMS_LOGGER_H_

#include <spdlog/sinks/stdout_color_sinks.h>

// Logs everything if necessary, uses pseudo-singleton structure
// Init() should be called before using
// Example:
//    Logger::get()->Init();
//    Logger::get()->debug(...);  // or warn(), info(), error(), etc.
class Logger {
 public:
    static void Init() {
        SetLevel(spdlog::level::debug);
        spdlog::stdout_color_st("Nonograms");  // init single-threaded
    }

    static void SetLevel(spdlog::level::level_enum log_level) {
        spdlog::set_level(log_level);
    }

    static auto get() {
        return spdlog::get("Nonograms");
    }
};

#endif  // NONOGRAMS_LOGGER_H_
