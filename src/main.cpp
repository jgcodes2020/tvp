#include "terminal.hpp"
#include "vdec.hpp"

#include <fmt/core.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <span>

#include "ffmpeg.hpp"

int main(int argc, char* argv[]) {
  tvp::video_decoder dec(argv[1]);
  const AVFrame* frame;
  while ((frame = dec.next())) {
    fmt::print("Frame {}\n", dec.count());
  }
}