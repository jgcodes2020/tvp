# TVP (Terminal Video Player)
My attempt at creating a real-time Sixel-based video player with as much power as the terminals can handle.

The current version can deal with 6-bit grayscale on mlterm (mlterm seems to be the most powerful sixel terminal to my knowledge). While a colour encoder is possible, I have yet to implement one, as the colour rounding algorithm only works for grayscale images.

## Build instructions
Build as you would a normal CMake project. This project also requires Conan.
