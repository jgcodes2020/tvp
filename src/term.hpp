#ifndef _TERM_HPP_
#define _TERM_HPP_
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string_view>
#include <fmt/core.h>
#include <termios.h>

namespace term {
  inline cv::Size sixel_size() {
    #if 1
    winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return cv::Size {size.ws_col * 10, size.ws_row * 20};
    #else
    return cv::Size {80 * 10, 42 * 20};
    #endif
  }
  // Terminal control
  inline void set_alt_buffer(bool set) {
    fmt::print("{}", set? "\e[?1049h" : "\e[?1049l");
  }
  inline void set_decsdm(bool set) {
    fmt::print("{}", set? "\e[?80h" : "\e[?80l");
  }
  inline void cls() {
    fmt::print("\e[2J\e[3J\e[H");
  }
  
  
  void show(cv::InputArray arr);
  
  void encode_sixel(cv::InputArray arr);
  void encode_iterm2(cv::InputArray arr);
}
#endif