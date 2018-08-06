/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#include <string>

#include <args.hxx>


namespace cli_args {
args::ArgumentParser parser("This is a nonogram solver program.");

args::HelpFlag help(parser, "help", "Display help", {'h', "help"});

args::ValueFlag<std::string> inputPuzzle(parser, "path_to_puzzle",
        "Solve a puzzle", {'i', "input"});

args::ValueFlag<std::string> inputImage(parser, "path_to_image",
        "Convert an image to the puzzle", {'p', "in-image"});

args::ValueFlag<std::string> imageName(parser, "image_name",
        "The file name of the solved puzzle image", {'o', "output"},
        "result");

args::ValueFlag<int> scaleImage(parser, "scale_factor",
        "The scale factor of the result image", {'s', "scale"}, 2);

args::ValueFlag<std::string> benchmark(parser, "path_to_puzzles",
        "Launch a benchmark", {'x', "benchmark"});

args::ValueFlag<int> gif_frame_delay(parser, "gif_frame_delay",
        "Delay between frames in the gif image (in ms)",
        {"gfd", "gif-frame-delay"}, 100);

args::ValueFlag<int> gif_end_delay(parser, "gif_end_delay",
        "The last frame delay in the gif image (in ms)",
        {"ged", "gif-end-delay"}, 1000);

args::Flag black(parser, "black",
        "Solve white-black puzzle (the default is colored)",
        {'b', "black"});

args::Flag moves(parser, "moves",
        "Generate step by step images of the solution process", {'m', "moves"});

args::Flag extra_moves(parser, "extra_moves", "Generate step by step images "
        "of the solution process with more details", {"em", "extra-moves"});

args::Flag gif(parser, "gif",
        "Generate animation of the solution process", {'g', "gif"});

args::Flag cool(parser, "cool", "Generate a cool image", {'c', "cool"});

args::Flag empty(parser, "empty", "Don't save the image of the puzzle",
        {'e', "empty"});

args::Flag display(parser, "display", "Display the image instead of writing to"
        " file", {'d', "display"});
}  // namespace cli_args
