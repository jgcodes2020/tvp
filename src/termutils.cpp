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
    return {ws.ws_col, ws.ws_row};
  }
}