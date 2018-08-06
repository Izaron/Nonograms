/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#include <benchmark.h>

#include <dirent.h>

#include <algorithm>
#include <functional>
#include <set>
#include <utility>
#include <vector>

#include <args.hxx>
#include <arguments.h>
#include <logger.h>
#include <puzzle.h>
#include <timespan.h>

using std::greater;
using std::max;
using std::pair;
using std::set;
using std::string;
using std::vector;

bool Benchmark::Run(const string& path_to_puzzles) {
    Logger::get()->info("Starting a benchmark...");

    // Access all entry names
    DIR* dir = opendir(path_to_puzzles.c_str());
    vector<string> files;
    if (dir) {
        dirent* entry = readdir(dir);
        while (entry) {
            if (strcmp(entry->d_name, ".") != 0 &&
                    strcmp(entry->d_name, "..") != 0) {
                files.push_back(string(entry->d_name));
            }
            entry = readdir(dir);
        }
    } else {
        Logger::get()->error("Failed to open directory {}", path_to_puzzles);
        Logger::get()->error("Maybe you forgot to add a slash \'/\' after"
                " the path?");
        return false;
    }

    // The set of running times of the slowest files
    set<pair<double, string>, greater<pair<double, string>>> top_set;
    vector<double> all_running_times;

    int solved = 0;
    double time_summary = 0.0;
    double time_max = 0.0;
    Timespan ts;

    // Disable low-level log messages to more clean output
    Logger::SetLevel(spdlog::level::warn);

    for (const auto& file : files) {
        Logger::get()->debug("Solving... {}", file);

        // Write the progress of the benchmark
        // Warning - disabled at line 40
        if (solved > 0) {
            Logger::get()->info("PROGRESS: {}%, AVERAGE TIME: {}, MAX TIME: {},"
                    " solving puzzle {}",
                    static_cast<double>(solved)
                        / static_cast<double>(files.size()) * 100.0,
                    time_summary / static_cast<double>(solved), time_max, file);
        }

        // Measure running time
        ts.Peek();
        Puzzle puzzle;
        if (!puzzle.Solve(path_to_puzzles + file)) {
            Logger::get()->error("Failed benchmark on file {}", file);
            return false;
        }
        double result = ts.Peek(true);

        // Update statistics
        time_summary += result;
        time_max = max(time_max, result);
        solved++;

        // Update the top set
        top_set.insert({result, file});
        if (top_set.size() > kMaxTopSize) {
            top_set.erase(*top_set.rbegin());
        }

        all_running_times.push_back(result);
    }

    // Revert the logger back to the info level
    Logger::SetLevel(spdlog::level::info);

    sort(all_running_times.begin(), all_running_times.end());

    // Print statistics
    Logger::get()->info("Average time: {}, Median time: {}, Max time: {}",
            time_summary / static_cast<double>(solved),
            all_running_times[all_running_times.size() / 2], time_max);

    Logger::get()->info("Top {} heaviest nonograms:", top_set.size());
    for (const auto& it : top_set) {
        Logger::get()->info("{} seconds, file {}", it.first, it.second);
    }

    return true;
}
