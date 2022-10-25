
#include "av.h"
#include "ppm.hpp"
#include "vdec.hpp"
#include <fmt/core.h>

int main(int argc, char* argv[]) {
  av::init();
  
  tvp::video_decoder vdec(argv[1]);
  
  for (int i = 0; i < 99; i++) {
    vdec.next();
  }
}