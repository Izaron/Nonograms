To build and run the program:

```
mkdir build
cd build
cmake ..
make
./nonogram_solver {OPTIONS}
```

Program options:

```
  ./nonograms_solver {OPTIONS}

    This is a nonogram solver program.

  OPTIONS:

      -h, --help                        Display help
      -i[path_to_puzzle],
      --input=[path_to_puzzle]          Solve a puzzle
      -p[path_to_image],
      --in-image=[path_to_image]        Convert an image to the puzzle
      -o[image_name],
      --output=[image_name]             The file name of the solved puzzle image
      -s[scale_factor],
      --scale=[scale_factor]            The scale factor of the result image
      -x[path_to_puzzles],
      --benchmark=[path_to_puzzles]     Launch a benchmark
      --gfd=[gif_frame_delay],
      --gif-frame-delay=[gif_frame_delay]
                                        Delay between frames in the gif image
                                        (in ms)
      --ged=[gif_end_delay],
      --gif-end-delay=[gif_end_delay]   The last frame delay in the gif image
                                        (in ms)
      -b, --black                       Solve white-black puzzle (the default is
                                        colored)
      -m, --moves                       Generate step by step images of the
                                        solution process
      --em, --extra-moves               Generate step by step images of the
                                        solution process with more details
      -g, --gif                         Generate animation of the solution
                                        process
      -c, --cool                        Generate a cool image
      -e, --empty                       Don't save the image of the puzzle
      -d, --display                     Display the image instead of writing to
                                        file
```

Warning - the function to write GIF files works buggy on my machine, you can try to remove this line in CMakeLists.txt:
```Cmake
add_definitions(-DBUGGY_MAGIC)  # can't write gif images
```
And then rebuild and run the program.

Additional libraries used in the project - [Magick++](https://github.com/ImageMagick/ImageMagick) and [args](https://github.com/Taywee/args). They may require the installation of some dependent libraries.
