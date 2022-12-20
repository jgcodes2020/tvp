#ifndef TVP_TERMINAL_HPP_INCLUDED
#define TVP_TERMINAL_HPP_INCLUDED

#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <ios>
#include <iostream>
#include <memory>
#include <ranges>
#include <span>
#include <stdexcept>
#include <system_error>
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

  class raw_input_guard {
  public:
    raw_input_guard() : m_tm(set_stty(true)) {}
    ~raw_input_guard() { restore_stty(m_tm); }

  private:
    termios m_tm;
  };

  inline std::pair<size_t, size_t> term_get_size() {
    winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

    size_t sx = (ws.ws_xpixel) ? ws.ws_xpixel : ws.ws_col * 8;
    size_t sy = (ws.ws_ypixel) ? ws.ws_ypixel : ws.ws_row * 10;

    return {sx, sy};
  }

  inline void pause() {
    raw_input_guard _g;
    std::cin.get();
    std::cout.put('\n');
  }
  
  inline bool check_sixel() {
    std::string s;
    {
      raw_input_guard _g;
      
      // Ask for terminal attributes.
      // This can hang on terminals that don't support
      // this particular escape sequence.
      std::cout << "\e[c" << std::flush;
      std::cin >> std::unitbuf;
      std::getline(std::cin, s, 'c');
      std::cin >> std::nounitbuf;
    }
    // Remove the fluff at the beginning of the return value.
    s.erase(0, 3);
    
    // split the string by semicolon. from SO:
    // https://stackoverflow.com/a/14267455/10808912
    size_t sp = 0;
    size_t ep = s.find_first_of(';');
    while (ep != std::string::npos) {
      if (std::stoul(s.substr(sp, ep - sp)) == 4)
        return true;
      sp = ep + 1;
      ep = s.find_first_of(';', sp);
    }
    
    return std::stoul(s.substr(sp)) == 4;
  }
}  // namespace tvp
#endif