/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#include <paint.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include <arguments.h>
#include <logger.h>
#include <puzzle.h>

#include <Magick++.h>

using std::endl;
using std::get;
using std::map;
using std::max;
using std::ofstream;
using std::pair;
using std::string;
using std::to_string;
using std::vector;


const char* Paint::kImageExtension = ".png";
const char* Paint::kGifExtension = ".gif";
vector<Magick::Image> Paint::gif_frames_ = vector<Magick::Image>();


/* Helper functions */

void ImageDrawSquare(Magick::Image& image, const Magick::Color& color,
        int pos_x, int pos_y, int scale) {
    for (int i = pos_x * scale; i < (pos_x + 1) * scale; i++) {
        for (int j = pos_y * scale; j < (pos_y + 1) * scale; j++) {
            image.pixelColor(j, i, color);
        }
    }
}

void ImageOverlayLine(Magick::Image& image, double multiplier,
        int start_x, int start_y, int end_x, int end_y) {
    for (int i = start_x; i <= end_x; i++) {
        for (int j = start_y; j <= end_y; j++) {
            Magick::ColorRGB c = image.pixelColor(j, i);
            Magick::ColorRGB result(c.red() * multiplier,
                    c.green() * multiplier, c.blue() * multiplier);
            image.pixelColor(j, i, result);
        }
    }
}

char GetHexSymbol(int v) {
    if (v >= 0 && v <= 9) {
        return static_cast<char>(v + '0');
    } else {
        return static_cast<char>(v - 10 + 'a');
    }
}

int ReverseColorByte(double d) {
    int res = 0;
    double error = d;
    for (int i = 1; i < 256; i++) {
        double value = static_cast<double>(i) / 256.0;
        double cur_error = fabs(d - value);
        if (cur_error < error) {
            error = cur_error;
            res = i;
        }
    }
    return res;
}

string ByteToHex(int v) {
    string hex_str;
    hex_str += GetHexSymbol(v / 16);
    hex_str += GetHexSymbol(v % 16);
    return hex_str;
}

string MagickColorToString(const Magick::ColorRGB& color) {
    int r = ReverseColorByte(color.red());
    int g = ReverseColorByte(color.green());
    int b = ReverseColorByte(color.blue());
    return "#" + ByteToHex(r) + ByteToHex(g) + ByteToHex(b);
}

int GetRandomNumber() {
    return 4;  // chosen by fair dice roll
               // guaranteed to be random
}

int GetColorIndex(map<string, int>& color_map, const Magick::Color& color) {
    string key = MagickColorToString(color);
    if (!color_map.count(key)) {
        int size = color_map.size();
        color_map[key] = size;
    }
    return color_map[key];
}

int GetIntFromHex(const string& hex_str) {
    int f, s;
    if (hex_str[0] >= '0' && hex_str[0] <= '9') {
        f = hex_str[0] - '0';
    } else {
        f = hex_str[0] - 'a' + 10;
    }
    if (hex_str[1] >= '0' && hex_str[1] <= '9') {
        s = hex_str[1] - '0';
    } else {
        s = hex_str[1] - 'a' + 10;
    }
    return f * 16 + s;
}

Magick::ColorRGB PuzzleColorToMagick(const Puzzle::Color& source) {
    double r = static_cast<double>(get<0>(source)) / 255.0;
    double g = static_cast<double>(get<1>(source)) / 255.0;
    double b = static_cast<double>(get<2>(source)) / 255.0;
    return Magick::ColorRGB(r, g, b);
}

/* Public functions */

void Paint::WriteImage(Magick::Image& image, int image_counter) {
    if (cli_args::display) {
        if (image_counter) {
            Logger::get()->info("Display image, count {}", image_counter);
        } else {
            Logger::get()->info("Display image");
        }
        image.display();
    } else {
        string filename = GetImageFilename(image_counter);
        Logger::get()->info("Write image to {}", filename);
        image.write(filename);
    }
}

string Paint::GetImageFilename(int image_counter) {
    string filename = args::get(cli_args::imageName);

    string counter = to_string(image_counter);
    while (counter.size() < GetRandomNumber()) {
        counter = "0" + counter;
    }
    filename += counter + string(kImageExtension);
    return filename;
}

string Paint::GetGifFilename() {
    string filename = args::get(cli_args::imageName);
    filename += string(kGifExtension);
    return filename;
}

vector<Magick::ColorRGB> ConvertPuzzleColorsToMagic(
        const vector<Puzzle::Color>& colors) {
    vector<Magick::ColorRGB> magic_colors(colors.size());
    for (int i = 0; i < colors.size(); i++) {
        magic_colors[i] = PuzzleColorToMagick(colors[i]);
    }
    return magic_colors;
}

Magick::Image Paint::CreateImage(const Puzzle::Config& config) {
    auto& n = config.n;
    auto& m = config.m;
    auto& colors = config.colors;
    auto& row_masks = config.row_masks;

    // Parse puzzle colors to Magick format
    auto magic_colors = ConvertPuzzleColorsToMagic(colors);

    // Draw the image
    Magick::Image image(Magick::Geometry(m, n), "white");
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < m; col++) {
            int mask = row_masks[row][col];
            if (__builtin_popcount(mask) > 1) {
                image.pixelColor(col, row, "black");
            } else {
                int color_index = __builtin_ctz(mask);
                image.pixelColor(col, row, magic_colors[color_index]);
            }
        }
    }

    // scale the image if needed
    if (cli_args::scaleImage) {
        int sc = args::get(cli_args::scaleImage);
        image.scale(Magick::Geometry(sc * m, sc * n));
    }

    return image;
}

void Paint::DrawImage(const Puzzle::Config& config, int image_count) {
    // check for the blocking flag
    if (cli_args::empty) {
        Logger::get()->info("Don't save the image");
        return;
    }

    auto image = CreateImage(config);

    WriteImage(image, image_count);
}

Magick::Image Paint::CreateCoolImage(const Puzzle::Config& config) {
    auto& n = config.n;
    auto& m = config.m;
    auto& colors = config.colors;
    auto& row_groups = config.row_groups;
    auto& col_groups = config.col_groups;
    auto& row_masks = config.row_masks;

    // Parse puzzle colors to Magick format
    auto magic_colors = ConvertPuzzleColorsToMagic(colors);

    auto color_border = PuzzleColorToMagick(Puzzle::ParseColor("#8f8f8f"));
    auto color_outer_background = PuzzleColorToMagick(
            Puzzle::ParseColor("#d7d7d7"));
    auto color_inner_background = PuzzleColorToMagick(
            Puzzle::ParseColor("#ffffff"));

    int pixelize = 2;
    if (cli_args::scaleImage) {
        pixelize = args::get(cli_args::scaleImage);
    }

    int max_group_count_horizontal = 0;
    int max_group_count_vertical = 0;

    for (const auto& groups : row_groups) {
        max_group_count_horizontal = max(max_group_count_horizontal,
                static_cast<int>(groups.size()));
    }

    for (const auto& groups : col_groups) {
        max_group_count_vertical = max(max_group_count_vertical,
                static_cast<int>(groups.size()));
    }

    int image_width = 1 + /* border */
                max_group_count_horizontal + /* outer background */
                1 + /* border */
                m + /* config of vertical groups */
                1;  /* border */

    int image_height = 1 + /* border */
                max_group_count_vertical + /* outer background */
                1 + /* border */
                n + /* config of horizontal groups */
                1;  /* border */

    // init image
    Magick::Image image(Magick::Geometry(image_width * pixelize,
                image_height * pixelize),
                "white");
    image.type(Magick::TrueColorType);

    // draw horizontal groups
    for (int i = 0; i < row_groups.size(); i++) {
        int pos = max_group_count_horizontal - row_groups[i].size() + 1;
        for (const auto& g : row_groups[i]) {
            ImageDrawSquare(image, magic_colors[g.second],
                    i + 2 + max_group_count_vertical, pos, pixelize);
            pos++;
        }
    }

    // draw vertical groups
    for (int i = 0; i < col_groups.size(); i++) {
        int pos = max_group_count_vertical - col_groups[i].size() + 1;
        for (const auto& g : col_groups[i]) {
            ImageDrawSquare(image, magic_colors[g.second], pos,
                    i + 2 + max_group_count_horizontal, pixelize);
            pos++;
        }
    }

    // draw image solution
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < m; col++) {
            int mask = row_masks[row][col];
            if (__builtin_popcount(mask) > 1) {
                continue;  // Draw nothing if haven't solved this pixel
            }
            int color_index = __builtin_ctz(mask);
            ImageDrawSquare(image, magic_colors[color_index],
                    row + 2 + max_group_count_vertical,
                    col + 2 + max_group_count_horizontal, pixelize);
        }
    }

    // draw separators
    // soft separators
    for (int row = 0; row < n; row++) {
        ImageOverlayLine(image, 0.9f, pixelize *
                    (row + 2 + max_group_count_vertical),
                0, pixelize * (row + 2 + max_group_count_vertical),
                image_width * pixelize - 1);
    }
    for (int col = 0; col < m; col++) {
        ImageOverlayLine(image, 0.9f, 0, pixelize *
                    (col + 2 + max_group_count_horizontal),
                image_height * pixelize - 1, pixelize *
                    (col + 2 + max_group_count_horizontal));
    }

    // group separators
    for (int i = 0; i < max_group_count_horizontal; i++) {
        ImageOverlayLine(image, 0.9f, pixelize * (2 + max_group_count_vertical),
                pixelize * (i + 1), image_height * pixelize - 1,
                pixelize * (i + 1));
    }
    for (int i = 0; i < max_group_count_vertical; i++) {
        ImageOverlayLine(image, 0.9f, pixelize * (i + 1),
                pixelize * (2 + max_group_count_horizontal), pixelize * (i + 1),
                image_width * pixelize - 1);
    }


    // 5-separators
    for (int row = 0; row < n; row += 5) {
        ImageOverlayLine(image, 0.9f, pixelize *
                    (row + 2 + max_group_count_vertical),
                0, pixelize * (row + 2 + max_group_count_vertical),
                image_width * pixelize - 1);
    }
    for (int col = 0; col < m; col += 5) {
        ImageOverlayLine(image, 0.9f, 0, pixelize *
                    (col + 2 + max_group_count_horizontal),
                image_height * pixelize - 1, pixelize *
                    (col + 2 + max_group_count_horizontal));
    }

    // draw borders
    for (int i = 0; i < image_height; i++) {
        // vertical borders
        ImageDrawSquare(image, color_border, i, 0, pixelize);
        ImageDrawSquare(image, color_border, i, max_group_count_horizontal + 1,
                pixelize);
        ImageDrawSquare(image, color_border, i, image_width - 1, pixelize);
    }
    for (int i = 0; i < image_width; i++) {
        // horizontal borders
        ImageDrawSquare(image, color_border, 0, i, pixelize);
        ImageDrawSquare(image, color_border, max_group_count_vertical + 1, i,
                pixelize);
        ImageDrawSquare(image, color_border, image_height - 1, i, pixelize);
    }

    // draw inner background
    for (int i = 1; i <= max_group_count_vertical; i++) {
        for (int j = 1; j <= max_group_count_horizontal; j++) {
            ImageDrawSquare(image, color_outer_background, i, j, pixelize);
        }
    }

    return image;
}

void Paint::DrawCoolImage(const Puzzle::Config& config, int image_counter) {
    // Check for the blocking flag
    if (cli_args::empty) {
        Logger::get()->info("Don't save the image");
        return;
    }

    auto image = CreateCoolImage(config);

    WriteImage(image, image_counter);
}

void Paint::PushCoolFrame(const Puzzle::Config& config) {
    // Check for the blocking flag
    if (cli_args::empty) {
        Logger::get()->info("Don't save the image");
        return;
    }

    auto image = CreateCoolImage(config);
    image.animationDelay(args::get(cli_args::gif_frame_delay));

    gif_frames_.push_back(image);
}

void Paint::PushFrame(const Puzzle::Config& config) {
    // Check for the blocking flag
    if (cli_args::empty) {
        Logger::get()->info("Don't save the image");
        return;
    }

    auto image = CreateImage(config);
    image.animationDelay(args::get(cli_args::gif_frame_delay));

    gif_frames_.push_back(image);
}

void Paint::ReleaseFrames() {
    if (!gif_frames_.empty()) {
        string filename = GetGifFilename();
        Logger::get()->info("Save gif image to {}", filename);
        gif_frames_.back().animationDelay(args::get(cli_args::gif_frame_delay));

        // We write a gif if yours Magick++ isn't buggy
#ifndef BUGGY_MAGIC
        Magick::writeImages(gif_frames_.begin(), gif_frames_.end(),
                filename, true);
#endif

        gif_frames_.clear();
    }
}

void Paint::EncodeImage(const string& image_path, const string& filename) {
    Magick::Image img;
    try {
        img.read(image_path);
    } catch (Magick::ErrorBlob& e) {
        Logger::get()->error(e.what());
        return;
    }

    ofstream fout(filename);

    map<string, int> color_indices;
    vector<vector<pair<int, int>>> row_groups(img.rows());
    vector<vector<pair<int, int>>> col_groups(img.columns());

    for (int i = 0; i < img.rows(); i++) {
        int cur_index = GetColorIndex(color_indices, img.pixelColor(0, i));
        int group_size = 1;
        vector<pair<int, int>> groups;
        for (int j = 1; j < img.columns(); j++) {
            int nxt_index = GetColorIndex(color_indices, img.pixelColor(j, i));

            double prev_alpha = static_cast<Magick::ColorRGB>(
                    img.pixelColor(j - 1, i)).alpha();
            double cur_alpha = static_cast<Magick::ColorRGB>(
                    img.pixelColor(j, i)).alpha();

            if (nxt_index != cur_index || prev_alpha != cur_alpha) {
                Magick::ColorRGB prev_color = img.pixelColor(j - 1, i);
                if (prev_color.alpha() != 0) {  // Is not transparent
                    groups.push_back({group_size, cur_index});
                }
                cur_index = nxt_index;
                group_size = 1;
            } else {
                group_size++;
            }
        }
        Magick::ColorRGB prev_color = img.pixelColor(img.columns() - 1, i);
        if (prev_color.alpha() != 0) {  // Is not transparent
            groups.push_back({group_size, cur_index});
        }
        row_groups[i] = groups;
    }

    for (int i = 0; i < img.columns(); i++) {
        int cur_index = GetColorIndex(color_indices, img.pixelColor(i, 0));
        int group_size = 1;
        vector<pair<int, int>> groups;
        for (int j = 1; j < img.rows(); j++) {
            int nxt_index = GetColorIndex(color_indices, img.pixelColor(i, j));

            double prev_alpha = static_cast<Magick::ColorRGB>(
                    img.pixelColor(i, j - 1)).alpha();
            double cur_alpha = static_cast<Magick::ColorRGB>(
                    img.pixelColor(i, j)).alpha();

            if (nxt_index != cur_index || prev_alpha != cur_alpha) {
                Magick::ColorRGB prev_color = img.pixelColor(i, j - 1);
                if (prev_color.alpha() != 0) {  // Is not transparent
                    groups.push_back({group_size, cur_index});
                }
                cur_index = nxt_index;
                group_size = 1;
            } else {
                group_size++;
            }
        }
        Magick::ColorRGB prev_color = img.pixelColor(i, img.rows() - 1);
        if (prev_color.alpha() != 0) {  // Is not transparent
            groups.push_back({group_size, cur_index});
        }
        col_groups[i] = groups;
    }

    // Put colors info
    vector<int> colors_red(color_indices.size());
    vector<int> colors_green(color_indices.size());
    vector<int> colors_blue(color_indices.size());
    fout << color_indices.size() << endl;
    for (int i = 0; i < color_indices.size(); i++) {
        for (const auto& it : color_indices) {
            if (it.second == i) {
                fout << it.first << endl;
                colors_red[i] = GetIntFromHex(it.first.substr(1, 2));
                colors_green[i] = GetIntFromHex(it.first.substr(3, 2));
                colors_blue[i] = GetIntFromHex(it.first.substr(5, 2));
                break;
            }
        }
    }
    fout << endl;

    // Put size
    fout << img.rows() << " " << img.columns() << endl << endl;

    // Put row groups
    for (int i = 0; i < img.rows(); i++) {
        fout << row_groups[i].size() << " ";
        for (const auto& it : row_groups[i]) {
            int length = it.first, index = it.second;
            fout << length << " " << colors_red[index] << " " <<
                colors_green[index] << " " << colors_blue[index] << " ";
        }
        fout << endl;
    }
    fout << endl;

    // Put col groups
    for (int i = 0; i < img.columns(); i++) {
        fout << col_groups[i].size() << " ";
        for (const auto& it : col_groups[i]) {
            int length = it.first, index = it.second;
            fout << length << " " << colors_red[index] << " " <<
                colors_green[index] << " " << colors_blue[index] << " ";
        }
        fout << endl;
    }
    fout << endl;

    fout.close();
}

