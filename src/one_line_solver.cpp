/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#include <one_line_solver.h>

#include <algorithm>
#include <sstream>
#include <utility>

#include <logger.h>

using std::pair;
using std::stringstream;
using std::vector;

bool OneLineSolver::Init(int side_length, int color_count) {
    if (!CheckMaxColorsOverflow(color_count)) {
        return false;
    }
    CheckMaxPreferredColorsOverflow(color_count);
    AllocMemory(side_length);
    return true;
}

bool OneLineSolver::CheckMaxColorsOverflow(int color_count) {
    if (color_count > kMaxColorsCount) {
        Logger::get()->error("Can't work with so many colors - {}",
                color_count);
        Logger::get()->error("Please don't have more than {} colors",
                kMaxColorsCount);
        return false;
    }
    return true;
}

void OneLineSolver::CheckMaxPreferredColorsOverflow(int color_count) {
    if (color_count > kMaxPreferredColorsCount) {
        Logger::get()->warn("Too many colors - {}", color_count);
        Logger::get()->warn("It's recommended to have no more than {} colors",
                kMaxPreferredColorsCount);
    }
}

void OneLineSolver::AllocMemory(int side_length) {
    cache_.resize(side_length + 1);
    calculated_fill_.resize(side_length + 1);
    for (int i = 0; i < cache_.size(); ++i) {
        cache_[i].resize(side_length + 1);
        calculated_fill_[i].resize(side_length + 1);
    }
    result_cells_.resize(side_length);
    cache_count_ = 0;
}

bool OneLineSolver::CanPlaceColor(const vector<int>& cells, int color,
        int lbound, int rbound) {
    // Went out of the border
    if (rbound >= cells.size()) {
        return false;
    }

    // We can paint a block of cells with a certain color if and only if it is
    // possible for all cells to have this color (that means, if every cell
    // from the block has color-th bit set to 1)
    int mask = 1 << color;
    for (int i = lbound; i <= rbound; ++i) {
        if (!(cells[i] & mask)) {
            return false;
        }
    }
    return true;
}

void OneLineSolver::SetPlaceColor(int color, int lbound, int rbound) {
    // Every cell from the block now can have this color
    for (int i = lbound; i <= rbound; ++i) {
        result_cells_[i] |= (1 << color);
    }
}

bool OneLineSolver::CanFill(const vector<pair<int, int>>& groups,
        const vector<int>& cells, int current_group = 0, int current_cell = 0) {
    // If we reached the end of the puzzle, all the groups should have
    // been placed
    if (current_cell == cells.size()) {
        return current_group == groups.size();
    }

    // Look at the cache to avoid useless recalculations
    int cached = cache_[current_group][current_cell];
    int answer = calculated_fill_[current_group][current_cell];
    if (cached == cache_count_) {
        return answer;
    }
    answer = 0;

    // Try to place a WHITE cell (0-th color)
    if (CanPlaceColor(cells, 0, current_cell, current_cell) &&
            CanFill(groups, cells, current_group, current_cell + 1)) {
        SetPlaceColor(0, current_cell, current_cell);
        answer = 1;
    }

    // Try to place CURRENT GROUP COLOR cells
    if (current_group < groups.size()) {
        int current_color = groups[current_group].second;
        int lbound = current_cell;
        int rbound = current_cell + groups[current_group].first - 1;

        bool can_place = CanPlaceColor(cells, current_color, lbound, rbound);
        bool place_white = false;  //  It may be required to place a white cell
                                   //  right after the current group
        int next_cell = rbound + 1;  // The cell to continue solve the puzzle

        // Check whether we are to put a white cell after the group
        if (can_place) {
            // If the next group color is the same, then we should place
            // a WHITE cell
            if (current_group + 1 < groups.size() &&
                    groups[current_group + 1].second == current_color) {
                place_white = true;
                can_place = CanPlaceColor(cells, 0, next_cell, next_cell);
                next_cell++;
            }
        }

        if (can_place) {
            // If after placement the puzzle can be solved, remember this
            if (CanFill(groups, cells, current_group + 1, next_cell)) {
                answer = 1;
                SetPlaceColor(current_color, lbound, rbound);
                if (place_white) {
                    SetPlaceColor(0, rbound + 1, rbound + 1);
                }
            }
        }
    }

    // Save cache and return
    calculated_fill_[current_group][current_cell] = answer;
    cache_[current_group][current_cell] = cache_count_;
    return answer;
}

void OneLineSolver::DebugLog(const std::vector<std::pair<int, int>>& groups,
        const std::vector<int>& cells) {
    // Log groups
    stringstream ss;
    ss << "Groups: ";
    for (const auto& it : groups) {
        ss << "(" << it.first << ", " << it.second << ") ";
    }
    Logger::get()->debug(ss.str());
    ss.str("");  // clear stream

    // Log cells
    ss << "Cells: ";
    for (const auto& it : groups) {
        ss << "(" << it.first << ", " << it.second << ") ";
    }
    Logger::get()->debug(ss.str());
}

bool OneLineSolver::UpdateState(const vector<std::pair<int, int>>& groups,
        vector<int>& cells) {
    // Update memory
    cache_count_++;
    fill(result_cells_.begin(), result_cells_.begin() + cells.size(), 0);

    if (!CanFill(groups, cells)) {
        Logger::get()->error("The puzzle can't be solved due to an incorrect "
                "input");
        DebugLog(groups, cells);
        return false;
    }

    // result_cells_ contains the updated state
    copy(result_cells_.begin(), result_cells_.begin() + cells.size(),
            cells.begin());
    return true;
}
