
#include "av.h"
#include "dither.hpp"
#include "frame.h"
#include "ppm.hpp"
#include "terminal.hpp"
#include "vdec.hpp"
#include "videorescaler.h"

#include <fmt/core.h>
#include <cstring>
#include <fstream>
#include <memory>

void test_quantize() {
  std::cout << "Input a hex code: ";
  uint32_t hex;
  std::cin >> std::hex >> hex;
  
  uint32_t out = tvp::truncate_xterm(hex);
  uint32_t outc = tvp::xterm_256[out];
  
  std::cout << out << " " << std::hex << outc << '\n';
}
void test_pixel_art() {
  av::VideoFrame frm(AV_PIX_FMT_RGB32, 16, 16);
  
  std::ofstream file("cols.txt");
  auto file_it = std::ostreambuf_iterator(file);
  
  for (uint32_t i : tvp::xterm_256) {
    fmt::format_to(file_it, "#{:6X}\n", i);
  }
}

int main(int argc, char* argv[]) {
  
  // while (true) {
  //   test_quantize();
  // }
  
  tvp::video_decoder vdec(argv[1]);
  av::VideoRescaler vrsc;
  av::VideoFrame frame, frame2;
  
  for (int i = 0; i < 99; i++) {
    frame = vdec.next();
    
    auto tsize = tvp::term_get_size();
    
    // frame2 = tvp::dither_frame(frame);
    // tvp::encode_ppm(frame2, "test.ppm");
    // tvp::pause();
  }
}