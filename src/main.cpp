#include "dither.hpp"
#include "ppm.hpp"
#include "terminal.hpp"
#include "video.hpp"

#include <fmt/core.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <span>

#include "ffmpeg.hpp"

int main(int argc, char* argv[]) {
  // if (!tvp::check_sixel()) {
  //   std::cout << "Oh noes, your terminal doesn't support Sixel! Use a terminal\n"
  //     << "that does support Sixel (like xterm -ti 430) and/or ask your\n"
  //     << "terminal's manufacturer/developer to add support. Thanks!\n";
  //   return 1;
  // }
  
  if (argc != 2) {
    return 2;
  }
  
  const AVFrame* frame0;
  av::ff_ptr<AVFrame> frame1(av::check_alloc(av_frame_alloc()));
  frame1->format = AV_PIX_FMT_RGB24;
  av::ff_ptr<AVFrame> frame2(av::check_alloc(av_frame_alloc()));
  frame2->format = AV_PIX_FMT_PAL8;
  av::ff_ptr<AVFrame> frame3(av::check_alloc(av_frame_alloc()));
  frame3->format = AV_PIX_FMT_RGB24;
  
  tvp::video_decoder dec(argv[1]);
  tvp::video_scaler scl0;
  tvp::video_scaler scl1;
  tvp::dither_context dth0;
  
  while ((frame0 = dec.next())) {
    av::frame_realloc_buffers(frame1.get(), frame0->width, frame0->height);
    av::frame_realloc_buffers(frame2.get(), frame0->width, frame0->height);
    av::frame_realloc_buffers(frame3.get(), frame0->width, frame0->height);
    
    tvp::init_palette(frame2.get());
    
    scl0.rescale(frame0, frame1.get());
    puts("rescale to RGB0");
    dth0.dither(frame1.get(), frame2.get());
    puts("dither");
    scl1.rescale(frame2.get(), frame3.get());
    puts("depalettize to RGB24");
    tvp::save_ppm("out.ppm", frame3.get());
    puts("write PPM");
    tvp::pause();
  }
}