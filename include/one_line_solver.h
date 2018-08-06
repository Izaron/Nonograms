/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef NONOGRAMS_ONE_LINE_SOLVER_H_
#define NONOGRAMS_ONE_LINE_SOLVER_H_

#include <utility>
#include <vector>

// Updates the state of an one-line colored Japan puzzle, given necessary
// groups description and the current cells state.
//
// Every group has its non-white color index and length, and expressed
// via std::pair
//
// Every cell of a puzzle is presented as an int value. If it's possible for
// a cell to have i-th color,  its i-th bit in the value is set to 1.
//
// Usually the count of colors doesn't exceed 11 (white + 10 additional),
// though with uint32 it may have 32 colors.
class OneLineSolver {
 public:
    // Checks color count, reserves memory
    bool Init(int side_length, int color_count);

    // Recalculates the state of a line, updating the values of the cell vector
    // returns false if the puzzle is unsolvable or has wrong state
    // Passing by reference is faster than returning some intermediate info
    bool UpdateState(const std::vector<std::pair<int, int>>& groups,
            std::vector<int>& cells);

 private:
    const int kMaxColorsCount = 31;
    const int kMaxPreferredColorsCount = 11;

    bool CheckMaxColorsOverflow(int color_count);
    void CheckMaxPreferredColorsOverflow(int color_count);
    void AllocMemory(int side_length);

    // Determines the possibility of filling the cells interval [lbound..rbound]
    // with a certain color
    bool CanPlaceColor(const std::vector<int>& cells, int color, int lbound,
            int rbound);

    // Remembers that there is a correct puzzle filling, where the interval
    // [lbound..rbound] is filled with a certain color
    void SetPlaceColor(int color, int lbound, int rbound);

    // Calling CanFill(G, C, X, Y) shows if it's possible to reach the end of
    // the puzzle, if we have placed X groups from G and currently are on the
    // Y-th cell from C.
    bool CanFill(const std::vector<std::pair<int, int>>& groups,
            const std::vector<int>& cells, int current_group,
            int current_cell);

    // Debug logs of groups and cells
    void DebugLog(const std::vector<std::pair<int, int>>& groups,
            const std::vector<int>& cells);

    // Used to save the last UpdateState() call count, when the [X][Y] element
    // was calculated, and protects from recalculations
    std::vector<std::vector<int>> cache_;

    // Used to save the information - the [X][Y] element shows if it's possible
    // to reach the end of the puzzle, if we have placed X groups and currently
    // are on the Y-th cell. A vector of bool isn't used due to its slowness.
    std::vector<std::vector<int>> calculated_fill_;

    // Used to save intermediate results (cells bit masks) before updating
    // the cells vector
    std::vector<int> result_cells_;

    //  Used to manage recalculations, increments when UpdateState() is called
    int cache_count_;
};

#endif  // NONOGRAMS_ONE_LINE_SOLVER_H_
