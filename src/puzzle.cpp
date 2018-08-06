/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#include <puzzle.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <thread>
#include <utility>

#include <arguments.h>
#include <logger.h>
#include <one_line_solver.h>
#include <paint.h>

#include <Magick++.h>

using std::ifstream;
using std::list;
using std::make_tuple;
using std::map;
using std::max;
using std::pair;
using std::string;
using std::vector;


Puzzle::Color Puzzle::ParseColor(const string& hex_color) {
    // #ff0f00 -> (255, 15, 0)
    if (hex_color.size() != 7 || hex_color[0] != '#') {
        Logger::get()->warn("Strange color found: {}", hex_color);
        Logger::get()->warn("Colors should look like \"#d7e41a\"");
        return make_tuple(0, 0, 0);
    }
    uint8_t r = stoul(hex_color.substr(1, 2), nullptr, 16);
    uint8_t g = stoul(hex_color.substr(3, 2), nullptr, 16);
    uint8_t b = stoul(hex_color.substr(5, 2), nullptr, 16);
    return make_tuple(r, g, b);
}

bool Puzzle::ReadColorsFromStream(ifstream& fin) {
    if (!(fin >> config_.color_count)) {
        return false;
    }

    config_.color_count++;  // Add default white color
    config_.colors.resize(config_.color_count);
    for (int i = 0; i < config_.color_count; i++) {
        string color;
        if (i > 0) {
            if (!(fin >> color)) {
                return false;
            }
        } else {
            // The first color is always white
            color = "#ffffff";
        }
        config_.colors[i] = ParseColor(color);
    }

    return true;
}

vector<vector<pair<int, int>>> Puzzle::ReadGroupInfoColored(ifstream& fin,
        map<Color, int>& color_map, int length) {
    vector<vector<pair<int, int>>> res(length);
    for (int row = 0; row < length; row++) {
        int group_size;
        if (!(fin >> group_size)) {
            return {};
        }
        if (group_size) {
            res[row].resize(group_size);
            for (int group = 0; group < group_size; group++) {
                int length, r, g, b;
                if (!(fin >> length >> r >> g >> b)) {
                    return {};
                }
                res[row][group] = {length, color_map[make_tuple(r, g, b)]};
            }
        }
    }
    return res;
}

vector<vector<pair<int, int>>> Puzzle::ReadGroupInfoBlack(ifstream& fin,
        int length) {
    vector<vector<pair<int, int>>> res(length);
    for (int row = 0; row < length; row++) {
        int group_size;
        if (!(fin >> group_size)) {
            return {};
        }
        if (group_size) {
            res[row].resize(group_size);
            for (int group = 0; group < group_size; group++) {
                int length;
                if (!(fin >> length)) {
                    return {};
                }
                res[row][group] = {length, 1};  // 1 is BLACK color
            }
        }
    }
    return res;
}

bool Puzzle::ReadColored(const string& filename) {
    ifstream fin(filename);

    if (!fin) {
        Logger::get()->error("Can't open file {}", filename);
        return false;
    }

    map<Color, int> color_indices;

    // Read colors
    if (!ReadColorsFromStream(fin)) {
        Logger::get()->error("Error while reading colors from file");
        return false;
    }

    for (int i = 0; i < config_.color_count; i++) {
        color_indices[config_.colors[i]] = i;
    }

    // Read the table
    if (!(fin >> config_.n >> config_.m)) {
        Logger::get()->error("Can't read the puzzle dimensions");
        return false;
    }

    config_.row_groups = ReadGroupInfoColored(fin, color_indices, config_.n);
    if (config_.row_groups.empty()) {
        Logger::get()->error("Can't read the puzzle rows groups");
        return false;
    }

    config_.col_groups = ReadGroupInfoColored(fin, color_indices, config_.m);
    if (config_.col_groups.empty()) {
        Logger::get()->error("Can't read the puzzle columns groups");
        return false;
    }

    return true;
}

bool Puzzle::ReadBlack(const string& filename) {
    ifstream fin(filename);

    if (!fin) {
        Logger::get()->error("Can't open file {}", filename);
        return false;
    }

    // Set colors (only two - white and black)
    config_.color_count = 2;
    config_.colors.resize(config_.color_count);
    config_.colors[0] = ParseColor("#ffffff");
    config_.colors[1] = ParseColor("#000000");

    // Read the table
    if (!(fin >> config_.n >> config_.m)) {
        Logger::get()->error("Can't read the puzzle dimensions");
        return false;
    }

    config_.row_groups = ReadGroupInfoBlack(fin, config_.n);
    if (config_.row_groups.empty()) {
        Logger::get()->error("Can't read the puzzle rows groups");
        return false;
    }

    config_.col_groups = ReadGroupInfoBlack(fin, config_.m);
    if (config_.col_groups.empty()) {
        Logger::get()->error("Can't read the puzzle columns groups");
        return false;
    }

    return true;
}

bool Puzzle::CheckUniqieness() {
    auto& row_masks = config_.row_masks;
    for (int row = 0; row < config_.n; row++) {
        for (int col = 0; col < config_.m; col++) {
            // Check if the value has 1 bit
            if (__builtin_popcount(row_masks[row][col]) != 1) {
                Logger::get()->error("The puzzle has no analytical solution!");
                return false;
            }
        }
    }
    return true;
}

int64_t Puzzle::UpdateCellValues() {
    uint64_t sum = 0;
    auto& row_masks = config_.row_masks;
    auto& col_masks = config_.col_masks;
    for (int row = 0; row < config_.n; row++) {
        for (int col = 0; col < config_.m; col++) {
            row_masks[row][col] &= col_masks[col][row];
            col_masks[col][row] &= row_masks[row][col];
            sum += row_masks[row][col];
        }
    }
    return sum;
}

void Puzzle::DrawImage() {
    if (cli_args::gif) {
        list<Magick::Image> gif_frames_;
        if (cli_args::cool) {
            Paint::PushCoolFrame(config_);
        } else {
            Paint::PushFrame(config_);
        }
    } else {
        if (cli_args::cool) {
            Paint::DrawCoolImage(config_, image_count_++);
        } else {
            Paint::DrawImage(config_, image_count_++);
        }
    }
}

bool Puzzle::UpdateGroupsState(OneLineSolver& solver, vector<int8_t>& dead,
        vector<vector<pair<int, int>>>& groups, vector<vector<int>>& masks) {
    int len = groups.size();

    int extra_move_count = 0;

    for (int i = 0; i < len; i++) {
        if (!dead[i]) {
            if (cli_args::extra_moves) {
                UpdateCellValues();
                extra_move_count = 0;
                for (const auto& it : masks[i]) {
                    if (__builtin_popcount(it) == 1) {
                        extra_move_count++;
                    }
                }
            }

            if (!solver.UpdateState(groups[i], masks[i])) {
                Logger::get()->error("Can't update the puzzle group state {}",
                        config_.filename);
                return false;
            }

            // A row is dead when all cells have known colors
            bool is_dead = true;
            for (auto num : masks[i]) {
                if (__builtin_popcount(num) != 1) {
                    is_dead = false;
                    break;
                }
            }
            dead[i] = is_dead;

            // Draw an extra image if needed
            if (cli_args::extra_moves) {
                UpdateCellValues();
                int current_count = 0;
                for (const auto& it : masks[i]) {
                    if (__builtin_popcount(it) == 1) {
                        current_count++;
                    }
                }

                if (current_count != extra_move_count) {
                    DrawImage();
                }
            }
        }
    }
    return true;
}

bool Puzzle::UpdateState(OneLineSolver& solver, vector<int8_t>& dead_rows,
        vector<int8_t>& dead_cols) {
    auto& row_masks = config_.row_masks;
    auto& col_masks = config_.col_masks;
    auto& row_groups = config_.row_groups;
    auto& col_groups = config_.col_groups;

    if (!UpdateGroupsState(solver, dead_rows, row_groups, row_masks)) {
        return false;
    }


    if (!UpdateGroupsState(solver, dead_cols, col_groups, col_masks)) {
        return false;
    }

    return true;
}

bool Puzzle::Solve(const string& filename) {
    config_.filename = filename;
    image_count_ = 0;

    if (cli_args::black) {
        if (!ReadBlack(filename))  {
            Logger::get()->error("Can't read the black-white puzzle file {}",
                    filename);
            return false;
        }
    } else {
        if (!ReadColored(filename))  {
            Logger::get()->error("Can't read the colored puzzle file {}",
                    filename);
            return false;
        }
    }

    int n = config_.n;
    int m = config_.m;
    int color_count = config_.color_count;
    auto& row_masks = config_.row_masks;
    auto& col_masks = config_.col_masks;

    // Initially, allow all colors for all cells
    row_masks.resize(n);
    for (int row = 0; row < n; row++) {
        row_masks[row].resize(m, (1 << color_count) - 1);
    }

    col_masks.resize(m);
    for (int col = 0; col < m; col++) {
        col_masks[col].resize(n, (1 << color_count) - 1);
    }

    // Solve the puzzle line by line
    OneLineSolver solver;
    solver.Init(max(n, m), color_count);

    vector<int8_t> dead_rows(n);
    vector<int8_t> dead_cols(m);

    int64_t prev_sum = LLONG_MAX;
    while (true) {
        // Draw the current step if needed
        if (cli_args::moves) {
            DrawImage();
        }

        if (!UpdateState(solver, dead_rows, dead_cols)) {
            Logger::get()->error("Can't update the puzzle state {}", filename);
            return false;
        }

        int64_t curr_sum = UpdateCellValues();
        if (curr_sum == prev_sum) {
            Logger::get()->info("The solution process has stopped");
            break;
        }

        prev_sum = curr_sum;
    }

    // Check for undeterministic result
    if (!CheckUniqieness()) {
        Logger::get()->error("Can't solve the puzzle {}", filename);
        return false;
    }

    DrawImage();
    return true;
}
