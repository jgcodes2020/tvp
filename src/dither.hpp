#ifndef TVP_DITHER_HPP_INCLUDED
#define TVP_DITHER_HPP_INCLUDED

#include <array>
#include <cstdint>

namespace tvp {
  extern const std::array<uint32_t, 256> xterm_256;
  
  uint32_t truncate_xterm(uint32_t x);
}
#endif