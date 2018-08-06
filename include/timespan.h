/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef NONOGRAMS_TIMESPAN_H_
#define NONOGRAMS_TIMESPAN_H_

#include <chrono>

// Calculates the running time of a function or an algorithm
//
// Peek() should be called before the piece of code to measure, and then
// after it (since Peek() returns the time passed from the previous call)
//
// Calling Peek() before the code may be omitted if it hasn't called ever.
// (The constructor saves the time the object was created)
//
// Example:
//    Timespan ts;
//    ts.Peek();  // this line can be omitted
//    do_a_cool_thing();
//    double seconds_passed = ts.Peek();
class Timespan {
 public:
    Timespan();
    double Peek(bool write_to_log = false);

 private:
    // Used to store the time point when Peek() was called last time
    std::chrono::high_resolution_clock::time_point prev_time_;
};

#endif  // NONOGRAMS_TIMESPAN_H_
