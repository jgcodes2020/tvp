#ifndef TVP_TERMINAL_HPP
#define TVP_TERMINAL_HPP

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <stdexcept>
#include <utility>

namespace tvp {
  inline termios set_stty(bool raw) {
    termios tm, tm_old;
    tcgetattr(STDOUT_FILENO, &tm);
    tm_old = tm;
    if (raw)
      tm.c_lflag &= ~(ECHO | ICANON);
    else
      tm.c_lflag |= (ECHO | ICANON);
    tcsetattr(STDOUT_FILENO, TCSANOW, &tm);
    return tm_old;
  }
  
  inline void restore_stty(const termios& tm) {
    tcsetattr(STDOUT_FILENO, TCSANOW, &tm);
  }
  
  inline std::pair<size_t, size_t> term_get_size() {
    winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    
    size_t sx = (ws.ws_xpixel)? ws.ws_xpixel : ws.ws_col * 8;
    size_t sy = (ws.ws_ypixel)? ws.ws_ypixel : ws.ws_row * 10;
    
    return {sx, sy};
  }
  
  inline void pause() {
    auto tm = set_stty(true);
    getchar();
    restore_stty(tm);
  }
  
  
}
#endif