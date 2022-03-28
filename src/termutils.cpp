#include "term.hpp"
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

term::term_size term::query_size() {
  if (!isatty(STDOUT_FILENO)) {
    return {80, 24};
  }
  else {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (ws.ws_xpixel && ws.ws_ypixel) {
      return {ws.ws_xpixel, ws.ws_ypixel};
    }
    else {
      return {size_t(ws.ws_col) * 10, size_t(ws.ws_row) * 20};
    }
  }
}