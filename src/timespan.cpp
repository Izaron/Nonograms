/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#include <timespan.h>

#include <iostream>

#include <logger.h>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

Timespan::Timespan() {
    prev_time_ = high_resolution_clock::now();
}

double Timespan::Peek(bool write_to_log) {
    auto cur_time = high_resolution_clock::now();
    auto time_span = duration_cast<duration<double>>(cur_time - prev_time_);
    double passed = time_span.count();

    if (write_to_log) {
        Logger::get()->info("Passed: {} seconds", passed);
    }

    prev_time_ = cur_time;
    return passed;
}
