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
  const AVFrame* frame0;
  AVFrame* frame1 = av::check_alloc(av_frame_alloc());
  frame1->format = AV_PIX_FMT_RGB0;
  AVFrame* frame2 = av::check_alloc(av_frame_alloc());
  frame2->format = AV_PIX_FMT_PAL8;
  AVFrame* frame3 = av::check_alloc(av_frame_alloc());
  frame3->format = AV_PIX_FMT_RGB24;
  
  tvp::video_decoder dec(argv[1]);
  tvp::video_scaler scl0;
  tvp::video_scaler scl1;
  
  while ((frame0 = dec.next())) {
    av::frame_realloc_buffers(frame1, frame0->width, frame0->height);
    av::frame_realloc_buffers(frame2, frame0->width, frame0->height);
    av::frame_realloc_buffers(frame3, frame0->width, frame0->height);
    
    scl0.rescale(frame0, frame1);
    puts("rescale to RGB0");
    tvp::dither(frame1, frame2);
    puts("dither");
    scl1.rescale(frame2, frame3);
    puts("depalettize to RGB24");
    tvp::save_ppm("out.ppm", frame3);
    puts("write PPM");
    tvp::pause();
  }
}