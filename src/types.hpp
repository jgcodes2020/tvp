#ifndef TVP_TYPES_HPP_INCLUDED
#define TVP_TYPES_HPP_INCLUDED

#include <cstdint>
#include <vector>

namespace tvp {
  struct colour {
    uint8_t r, g, b;
  };
  using palette = std::vector<colour>;
  
}

#endif