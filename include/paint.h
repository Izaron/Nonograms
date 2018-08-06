/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef NONOGRAMS_PAINT_H_
#define NONOGRAMS_PAINT_H_

#include <string>
#include <vector>

#include <puzzle.h>

namespace Magick {
class Image;
class ColorRGB;
}

// A set of functions used to manage images (display, encode, write)
class Paint {
 public:
    static void DrawCoolImage(const Puzzle::Config& config,
            int image_count = 0);
    static void DrawImage(const Puzzle::Config& config, int image_count = 0);

    static void PushCoolFrame(const Puzzle::Config& config);
    static void PushFrame(const Puzzle::Config& config);

    static void ReleaseFrames();

    static void EncodeImage(const std::string& image_path,
            const std::string& filename);

 private:
    static void WriteImage(Magick::Image& image, int image_counter);

    static Magick::Image CreateCoolImage(const Puzzle::Config& config);
    static Magick::Image CreateImage(const Puzzle::Config& config);

    static std::string GetImageFilename(int image_counter);
    static std::string GetGifFilename();

    static const char* kImageExtension;
    static const char* kGifExtension;

    // Used to manage GIF image frames
    static std::vector<Magick::Image> gif_frames_;
};


#endif  // NONOGRAMS_PAINT_H_
