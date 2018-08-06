/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef NONOGRAMS_BENCHMARK_H_
#define NONOGRAMS_BENCHMARK_H_

#include <string>

class Benchmark {
 public:
    bool Run(const std::string& path_to_puzzles);

 private:
    // The max size of the top of the slowest files
    const int kMaxTopSize = 10;
};

#endif  // NONOGRAMS_BENCHMARK_H_
