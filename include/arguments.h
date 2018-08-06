/* 
 * Copyright © 2018 Evgeny Shulgin <izaronplatz@gmail.com>
 * This code is released under the license described in the LICENSE file
 */
#ifndef NONOGRAMS_ARGUMENTS_H_
#define NONOGRAMS_ARGUMENTS_H_

#include <string>

#include <args.hxx>

// The constraints of the arguments parser library allow to handle arguments
// only via global values
namespace cli_args {
extern args::ArgumentParser parser;
extern args::HelpFlag help;
extern args::ValueFlag<std::string> inputPuzzle;
extern args::ValueFlag<std::string> inputImage;
extern args::ValueFlag<std::string> imageName;
extern args::ValueFlag<int> scaleImage;
extern args::ValueFlag<std::string> benchmark;
extern args::ValueFlag<int> gif_frame_delay;
extern args::ValueFlag<int> gif_end_delay;
extern args::Flag black;
extern args::Flag moves;
extern args::Flag extra_moves;
extern args::Flag gif;
extern args::Flag cool;
extern args::Flag empty;
extern args::Flag display;
}  // namespace cli_args

#endif
