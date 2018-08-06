/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef NONOGRAMS_PUZZLE_H_
#define NONOGRAMS_PUZZLE_H_

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <one_line_solver.h>

// Reads the puzzle from a file and solves it
class Puzzle {
 public:
    typedef std::tuple<uint8_t, uint8_t, uint8_t> Color;

    struct Config {
        std::string filename;
        int n;
        int m;
        int color_count;
        std::vector<Color> colors;
        std::vector<std::vector<std::pair<int, int>>> row_groups;
        std::vector<std::vector<std::pair<int, int>>> col_groups;
        std::vector<std::vector<int>> row_masks;
        std::vector<std::vector<int>> col_masks;
    };

    // Returns true if read correctly
    bool ReadColored(const std::string& filename);
    bool ReadBlack(const std::string& filename);
    // Returns true if solved successfully
    bool Solve(const std::string& filename);
    // Returns true if a new iteration of solution went correctly
    bool IterationSolve();
    // Parses strings like "#d7d7d7" to Color type
    static Color ParseColor(const std::string& hex_color);

 private:
    // Reads all the colors to config_
    bool ReadColorsFromStream(std::ifstream& fin);

    void DrawImage();

    bool UpdateState(OneLineSolver& solver, std::vector<int8_t>& dead_rows,
            std::vector<int8_t>& dead_cols);

    bool UpdateGroupsState(OneLineSolver& solver, std::vector<int8_t>& dead,
        std::vector<std::vector<std::pair<int, int>>>& groups,
        std::vector<std::vector<int>>& masks);

    // Updates cell values (both row and columns) and returns its sum
    int64_t UpdateCellValues();
    // Checks that the solution was unique
    bool CheckUniqieness();

    // Used to read vertical and horizontal colored groups
    std::vector<std::vector<std::pair<int, int>>> ReadGroupInfoColored(
            std::ifstream& fin, std::map<Color, int>& color_map, int length);
    // Used to read vertical and horizontal black and white groups
    std::vector<std::vector<std::pair<int, int>>> ReadGroupInfoBlack(
            std::ifstream& fin, int length);
    // Used to manage multi-image output
    int image_count_;

    Config config_;
};

#endif  // NONOGRAMS_PUZZLE_H_
