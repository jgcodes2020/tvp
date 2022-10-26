
#include "av.h"
#include "frame.h"
#include "ppm.hpp"
#include "terminal.hpp"
#include "vdec.hpp"
#include "videorescaler.h"
#include <fmt/core.h>
#include <libavutil/pixfmt.h>

int main(int argc, char* argv[]) {
  av::init();
  
  tvp::video_decoder vdec(argv[1]);
  av::VideoRescaler vrsc;
  av::VideoFrame frame;
  
  for (int i = 0; i < 99; i++) {
    frame = vdec.next();
    
    auto tsize = tvp::term_get_size();
    
    vrsc = av::VideoRescaler(tsize.first, tsize.second, AV_PIX_FMT_RGB32_1);
    frame = vrsc.rescale(frame, av::throws());
    
    
  }
}